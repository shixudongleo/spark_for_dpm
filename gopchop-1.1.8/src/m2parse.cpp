/*
#
# MPEG2Parser class testing tool
#
# $Id: m2parse.cpp 347 2009-03-02 23:27:14Z keescook $
#
# Copyright (C) 2001-2009 Kees Cook
# kees@outflux.net, http://outflux.net/
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
# http://www.gnu.org/copyleft/gpl.html
#
*/

#include "GOPchop.h"

#include <stdio.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/uio.h>
#include <string.h>

// Objects
#include "Bounds.h"
#include "List.h"
#include "ElementStream.h"
#include "Pack.h"
#include "MPEG2Parser.h"


// decode
#ifdef __cplusplus
extern "C"
{
#endif
#include <mpeg2dec/video_out.h>
#include <mpeg2dec/mpeg2.h>
#include <mpeg2dec/mm_accel.h>
#ifdef __cplusplus
}
#endif

// write to disk?
#undef WRITEOUT
// display to screen?
#undef WATCHOUT
// parse the file at all?
#define RENDEROUT


static mpeg2dec_t mpeg2dec;
static vo_open_t *output_open = NULL;
static vo_instance_t *output = NULL;


int debug = FALSE;
int verbose = FALSE;

void ticker(char *task, float done)
{
    printf("%s: %4.2f%%\r", task, done * 100.0);
    fflush(NULL);
}

void decode_init()
{
    int i;

    // decoding
    uint32_t accel = 0;
    vo_driver_t *drivers;

    drivers = vo_drivers();
    for (i = 0; drivers[i].name; i++)
        if (!strcmp(drivers[i].name, "xv"))
            output_open = drivers[i].open;

    if (!output_open)
    {
        fprintf(stderr, "%s", _("can't find 'xv' output\n"));
        exit(1);
    }

    accel = (mm_accel() | MM_ACCEL_MLIB);
    vo_accel(accel);
    output = vo_open(output_open);

    if (!output)
    {
        fprintf(stderr, "%s", _("can't open 'xv' output\n"));
        exit(1);
    }

    mpeg2_init(&mpeg2dec, accel, output);
}

// FIXME: maybe mmap isn't such a good idea for giant files?
int main(int argc, char *argv[])
{
    int i, out, count;
    MPEG2Parser *mpeg2parser;


    // check for our arguments
#ifdef WRITEOUT
    if (argc < 3)
    {
        fprintf(stderr, _("Usage: %s INPUT OUTPUT\n"), argv[0]);
        exit(1);
    }
#else // WRITEOUT
    if (argc < 2)
    {
        fprintf(stderr, _("Usage: %s INPUT\n"), argv[0]);
        exit(1);
    }
#endif // WRITEOUT

    decode_init();

    mpeg2parser = new MPEG2Parser();
    if (!mpeg2parser->init(argv[1], ticker))
    {
        fprintf(stderr, "%s\n", mpeg2parser->getError());
        exit(1);
    }

    mpeg2parser->parse();

    if (mpeg2parser->getError())
        fprintf(stderr, "%s\n", mpeg2parser->getError());

#ifdef WRITEOUT
    // open our output file for writing
    if ((out = open(argv[2], O_WRONLY | O_CREAT | O_EXCL | O_LARGEFILE,
                    S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0)
    {
        perror(argv[2]);
        exit(1);
    }
#endif // WRITEOUT


    List *GOPs = mpeg2parser->getGOPs();

    printf(_("GOPs: %d\n"), GOPs->getNum());

#ifdef RENDEROUT
    int GOPmax = GOPs->getNum();
    for (i = 0; i < GOPmax; i++)
    {
        Pack *packet;
        GroupOfPictures *GOP;
        Bounds *bounds;
        ElementStream *ves;
        int start, stop;
        int ves_start, ves_stop;

        uint8_t *loc;
        size_t len;

        ticker(_("Rendering GOPs"), (float) ((float) i / (float) GOPmax));

        GOP = (GroupOfPictures *) GOPs->vector(i);

        bounds = GOP->getPacketBounds();
        start = bounds->getFirst();
        stop = bounds->getMax();

        for (int j = start; j < stop; j++)
        {
            if (!(packet = (Pack *) mpeg2parser->getPackets()->vector(j)))
            {
                printf("%s", _("NULL packet?!\n"));
                exit(1);
            }

            ves_start = packet->getVideoFirst();
            ves_stop = packet->getVideoMax();

#ifdef WRITEOUT
            /* write packets to disk */
            if (loc =
                mpeg2parser->bytesAvail(packet->getStart(), packet->getLen()))
            {
                write(out, loc, packet->getLen());
            }
            else
                printf("%s", _("NULL vector area?!\n"));
#endif // WRITEOUT

            for (int k = ves_start; k < ves_stop; k++)
            {
                if (!
                    (ves =
                     (ElementStream *) mpeg2parser->getVideo()->vector(k)))
                {
                    printf("%s", _("NULL ves?!\n"));
                    exit(1);
                }
#ifdef WATCHOUT
                len = ves->getLen();
                (void *) loc = mpeg2parser->bytesAvail(ves->getStart(), len);
                if (loc && len)
                {
                    printf("%" OFF_T_FORMAT ": %ld\n", ves->getStart(), len);
                    fflush(NULL);
                    mpeg2_decode_data(&mpeg2dec, loc, loc + len);
                }
#endif // WATCHOUT
            }
        }
    }
    ticker("Rendering GOPs", 1.0);
#endif // RENDEROUT

    /*
       printf("%s",_("dumping audio ES...\n"));
       for (i=0;i<audio_list->getNum();i++)
       {
       ElementStream * audio;

       if (!(audio=(ElementStream*)audio_list->vector(i)))
       {
       Error("NULL audio vector?!");
       exit(1);
       }

       write(out,audio->getStart(),audio->getLen());
       }

       // close outfile
       close(out);

     */

    /*
       char user[512];

       printf(_("view GOPs avail: 0-%d\n"),pack_list->getNum()-1);
       printf("%s",_("Dump which GOP? "));
       fflush(NULL);
       while (fgets(user,512,stdin))
       {
       int which;
       int first, last;

       which=atoi(user);

       if (which<pack_list->getNum())
       {
       Pack * pack;
       ElementStream * video;

       if ((pack=(Pack*)pack_list->vector(which)))
       {
       first=pack->getMin();
       last=pack->getMax();

       printf(_("Showing ES %d to %d...\n"),
       first,last);
       for (i=first;i<last;i++)
       {

       if ((video=(ElementStream*)video_list->vector(i)))
       {

       mpeg2_decode_data(&mpeg2dec,
       video->getStart(),
       video->getStart()+
       video->getLen());
       }
       else
       Error("ES not found");
       }
       }
       else
       Error("Pack not found");
       }

       printf("%s",_("Dump which GOP? "));
       fflush(NULL);
       }
     */

#ifdef WRITEOUT
    close(out);
#endif

#if 0
    printf("%s", _("press enter to quit..."));
    fflush(NULL);
    char userin[128];
    fgets(userin, 128, stdin);
#endif

    // shutdown decoder
    mpeg2_close(&mpeg2dec);
    vo_close(output);

    // shutdown parser
    delete mpeg2parser;

    return 0;
}


/* vi:set ai ts=4 sw=4 expandtab: */
