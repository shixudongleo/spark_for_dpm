/*
#
# Worker for writing GOP information to files
#
# $Id: GOPutils.cpp 347 2009-03-02 23:27:14Z keescook $
#
# Copyright (C) 2003-2009 Kees Cook
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
#include "GOPutils.h"

#include "MPEG2Parser.h"
#include "Picture.h"

// display writing locations (from write_GOP) to stderr
#undef MPEG2_CLIPS_TO_STDERR

#include "mpeg2consts.h"
void write_timestamp(unsigned char * head, uint32_t pictures, int rate);

/*
 * This exports a text file that can be used to re-load an MPEG2-PS
 * much more quickly.  (And this file can be used to check the results
 * of GOPchop's parser.)
 *
 * Format is:
 *   TAG COUNT OFFSET[:LENGTH]
 *
 * GOP %u %llu:%lu
 * pack %u %llu
 * ...
 * pic %u %llu:%lu
 * ves %u %llu:%lu
 * ...
 *
 * If you have more GOPs than can be indexed by a 32bit variable,
 * then you can change the index variable yourself.  ;)
 *
 */
int write_gop_cache(char * filename, MPEG2Parser * mpeg2parser)
{
    int result=-1;
    List *GOPs;
    List *packets;
    List *pictures;
    List *ves;
    Vector *vector;
    uint32_t gopnum, gopmax;
    FILE * fp;

    if (!(fp=fopen(filename,"w"))) {
        perror(filename);
        return -1;
    }

    if (!(GOPs = mpeg2parser->getGOPs())) {
        printf("%s",_("NULL GOP list?!\n"));
        goto need_close;
    }
    if (!(packets = mpeg2parser->getPackets())) {
        printf("%s", _("NULL packet list?!\n"));
        goto need_close;
    }
    if (!(pictures = mpeg2parser->getPictures())) {
        printf("%s", _("NULL picture list?!\n"));
        goto need_close;
    }
    if (!(ves = mpeg2parser->getVideo())) {
        printf("%s", _("NULL VES list?!\n"));
        goto need_close;
    }

    gopmax=GOPs->getNum();
    for (gopnum=0;gopnum<gopmax;gopnum++) {
        GroupOfPictures *gop;
        Bounds *bounds;
        uint32_t num, max;
        Vector *header;

        if (!(gop=(GroupOfPictures*)GOPs->vector(gopnum))) {
            printf("%s", _("Missing GOP %u!?\n"),gopnum);
            goto need_close;
        }

        if (!(header=gop->getHeader())) {
            printf("%s",_("Missing GOP %u header!?\n"),gopnum);
            goto need_close;
        }

        fprintf(fp,"GOP %u %" OFF_T_FORMAT "\n",
                   gopnum, header->getStart());

        /* report packets */
        if (!(bounds=gop->getPacketBounds())) {
            printf("%s", _("NULL packet list for GOP %d?!\n"), gopnum);
            goto need_close;
        }
        max=bounds->getMax();
        for (num=bounds->getFirst();num<max;num++) {
            Pack *pack;

            if (!(pack=(Pack*)packets->vector(num))) {
                printf("%s",_("Missing Pack %u!?\n"),num);
                goto need_close;
            }

            fprintf(fp,"pack %u %" OFF_T_FORMAT "\n",
                       num, pack->getStart());
            
        }

        /* report pictures */
        if (!(bounds=gop->getPictureBounds())) {
            printf("%s", _("NULL picture list for GOP %d?!\n"), gopnum);
            goto need_close;
        }
        max=bounds->getMax();
        for (num=bounds->getFirst();num<max;num++) {
            Picture *picture;
            Bounds * vesbounds;
            uint32_t vesnum, vesmax;

            if (!(picture=(Picture*)pictures->vector(num))) {
                printf("%s",_("Missing Picture %u!?\n"),num);
                goto need_close;
            }

            fprintf(fp,"pic %u %" OFF_T_FORMAT "\n",
                       num,picture->getStart());

            /* report video */
            if (!(vesbounds=picture->getVideoBounds())) {
                printf("%s", _("NULL video ES list for GOP %d?!\n"), gopnum);
                goto need_close;
            }
            vesmax=vesbounds->getMax();
            for (vesnum=vesbounds->getFirst();vesnum<vesmax;vesnum++) {
                ElementStream *es;

                if (!(es=(ElementStream*)ves->vector(vesnum))) {
                    printf("%s",_("Missing VES packet %u!?\n"),vesnum);
                    goto need_close;
                }

                fprintf(fp,"ves %u %" OFF_T_FORMAT ":%lu\n",
                           vesnum,es->getStart(),es->getLen());
            }
        }
    }

    result=0;

need_close:
    if (fclose(fp)) {
        perror(filename);
        result=-1;
    }
    return result;
}

void write_clip_list(char * filename, struct clip_list_t * clips)
{
}

/*
 * Global memory area to do GOPheader manipulations
 */
uint8_t * GOPareaPtr = NULL;
size_t GOPareaSize = 0;
/*
 * file: stream to write to
 * mpeg2parser: object to read MPEG2 data from
 * num: which GOP to write
 * continuous: was the GOP before this one the "original" prior GOP?
 * drop_orphaned_frames: should non-continuous orphaned B/P frames get dropped?
 * adjust_timestamps: should timestamps be updated?
 * pictures_written: pointer to read/update number of pictures written
 *
 */
off_t write_GOP(FILE * file, MPEG2Parser * mpeg2parser,
                uint32_t num, int continuous,
                int drop_orphaned_frames,
                int adjust_timestamps,
                uint32_t *pictures_written,
                bool is_last_gop,
                bool drop_trailing_pack_with_system_header
                )
{
    List *GOPs;
    GroupOfPictures *GOP;
    Vector *GOPheader;
    List *packets;
    Bounds *packet_bounds;
    Bounds *picture_bounds;
    Vector *pes_vector;
    Pack *packet;
    uint32_t pes, pes_max;
    uint8_t old_bits;
    uint8_t *header;
    uint8_t *area;
    int P_Frame_seen;
    int open_GOP;

    off_t written_bytes;
    uint32_t written_pictures;

    size_t GOPlen;
    size_t GOPoffset;


    written_bytes = 0;
    written_pictures = 0;
    P_Frame_seen = 0;
    open_GOP = 0;

    if (!mpeg2parser || !file) return 0;

    if (!(GOPs = mpeg2parser->getGOPs())) {
        printf("%s", _("NULL GOP list?!\n"));
        return 0;
    }
    if (!(packets = mpeg2parser->getPackets())) {
        printf("%s", _("NULL packet list?!\n"));
        return 0;
    }
    if (!(GOP = (GroupOfPictures *) GOPs->vector(num))) {
        printf("%s", _("NULL GOP?!\n"));
        return 0;
    }
    if (!(GOPheader = (Vector *) GOP->getHeader())) {
        printf("%s", _("NULL GOP header?!\n"));
        return 0;
    }
    if (!(packet_bounds = GOP->getPacketBounds())) {
        printf("%s", _("NULL packet bounds?!\n"));
        return 0;
    }

    // figure out how many pictures we're going to write
    if (!(picture_bounds = GOP->getPictureBounds())) {
        printf("%s", _("NULL GOP picture bounds?!\n"));
        return 0;
    }
    written_pictures = picture_bounds->getMax() - picture_bounds->getFirst();

    // instead of writing one packet at a time, let's try
    // writing whole GOPs at once.

    // figure out size of GOP in memory
    GOPlen = 0;
    pes_max = packet_bounds->getMax();
    
    /* TI! */
    if (drop_trailing_pack_with_system_header &&
        is_last_gop) //check if the last pack has a system header
    {
        Pack *last_packet = (Pack *) packets->vector(pes_max - 1);  //last packet
        
        pack_header_t *pk_header = (pack_header_t *)mpeg2parser->bytesAvail(last_packet->getStart(), sizeof(pack_header_t));
        
        //is it here the system header?
        off_t addr_system_header= last_packet->getStart() + sizeof(pack_header_t) + (pk_header->bits[9] & 0x07);
        //printf("addr_system_header [%lu]\n",(unsigned long) addr_system_header);
        
        system_header_t *sys_header = (system_header_t *)
            mpeg2parser->bytesAvail(addr_system_header, sizeof(system_header_t));
        
        if (sys_header->start_code[0]==0x00 &&
            sys_header->start_code[1]==0x00 &&
            sys_header->start_code[2]==0x01 &&
            sys_header->start_code[3]==0xBB)
        {    
            //printf("Found system header in Last Pack\n");
            pes_max--;
        }
    }
    /* end TI! */
    
    for (pes = packet_bounds->getFirst(); pes < pes_max; pes++)
    {
        if (!(packet = (Pack *) packets->vector(pes)))
        {
            printf("%s", _("NULL PES?!\n"));
            return 0;
        }
        GOPlen += packet->getLen();
    }
    if (GOPlen > GOPareaSize)
    {
        // let's not realloc because we don't need to keep the
        // contents
        if (GOPareaPtr)
            free(GOPareaPtr);
        if (!(GOPareaPtr = (uint8_t *) malloc(GOPlen)))
        {
            printf("%s", _("Ran out of memory!?\n"));
            return 0;
        }
        GOPareaSize = GOPlen;
    }

    GOPoffset = 0;
    //pes_max = packet_bounds->getMax(); //TI!
    for (pes = packet_bounds->getFirst(); pes < pes_max; pes++)
    {
        size_t len;
        int undo;
        int okay_to_write;
        int has_header;

        // assume we can write this packet...
        okay_to_write = 1;
        has_header = 0;
        //printf("writing PES %d\n",pes);

        if (!(packet = (Pack *) packets->vector(pes)))
        {
            printf("%s", _("NULL PES?!\n"));
            goto abort;
        }

        len = packet->getLen();
        if (!(area = mpeg2parser->bytesAvail(packet->getStart(), len)))
        {
            printf("%s", _("unavailable memory?!\n"));
            goto abort;
        }

        // do our header verifications in mmap, but make the
        // actual changes in our shared memory area
        if (GOPheader->getStart() > packet->getStart() &&
            GOPheader->getStart() + GOPheader->getLen() <
            packet->getStart() + packet->getLen())
        {
            // GOP header is in this packet
            has_header = 1;
            header = area + (GOPheader->getStart() - packet->getStart());

            // do we have an open GOP?
            open_GOP = ((header[7] & 0x40) == 0);
        }

        // handle B-frame dropping
        if (drop_orphaned_frames && open_GOP && !continuous && !P_Frame_seen)
        {
            Vector *pic_header;

            // if this packet has a B-frame, skip it
            if ((pic_header =
                 mpeg2parser->findCode((Vector *) packet,
                                       (uint8_t*)picture_start_code,
                                       6)))
            {
                int pic_type;

                pic_type =
                    (area[pic_header->getStart() - packet->getStart() + 5] &
                     PICTURE_CODING_MASK) >> 3;

                if (pic_type == PICTURE_CODING_BIDIRECTIONAL)
                {
                    //printf("Dropping open GOP B-frame on splice\n");
                    okay_to_write = 0;
                    written_pictures--;
                }
                if (pic_type == PICTURE_CODING_PREDICTIVE)
                {
                    //printf("Keeping rest of frames in open GOP splice\n");
                    P_Frame_seen = 1;
                }
            }
        }

        if (okay_to_write)
        {
#ifdef MPEG2_CLIPS_TO_STDERR
            fprintf(stderr, _("\tWriting @ %llu for %d bytes\n"),
                    packet->getStart(), len);
#endif
            memcpy(GOPareaPtr + GOPoffset, area, len);

            /* handle making adjustments to GOP header */
            if (has_header)
            {
                // GOP header is in our current memory area
                header = GOPareaPtr + GOPoffset +
                            (GOPheader->getStart() - packet->getStart());

                if (!continuous && open_GOP) {
                    // need to set the "broken" flag
 
                    // an editor has been here
                    header[7] |= GOP_FLAG_BROKEN;   
                    // but most decoders don't care
                    if (drop_orphaned_frames)
                        header[7] |= GOP_FLAG_CLOSED;   
                    /*
                       printf("GOP: 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
                       header[0],header[1],header[2],header[3],
                       header[4],header[5],header[6],header[7]);
                     */
                }

                if (adjust_timestamps) {
                    struct sequence_info info;
                    // do it
                    GOP->get_sequence_info(&info);
                    write_timestamp(header,*pictures_written,info.frame_rate);
                }
            }

            GOPoffset += len;
        }
    }

    if (GOPoffset > 0)
    {
        // flush to disk
        if (fwrite(GOPareaPtr, sizeof(uint8_t), GOPoffset, file) != GOPoffset)
        {
            printf("%s", _("write failed\n"));
            goto abort;
        }
        written_bytes += GOPoffset;
        *pictures_written += written_pictures;
    }
  abort:
    return written_bytes;
}

void write_timestamp(unsigned char * head, uint32_t pictures, int rate) {
    uint32_t seconds, minutes, hours, top, remainder;

    //printf("pictures: %u rate: %s ", pictures,frame_str[rate]);

    /*
        rate = frames/second
        seconds = frames / rate
        frames = seconds * rate
     */

    seconds = pictures * rate_frac[rate].seconds / rate_frac[rate].frames;
    hours = (seconds/60)/60;
    minutes = (seconds/60) % 60;
    seconds = seconds % 60;
    pictures -= seconds * rate_frac[rate].frames / rate_frac[rate].seconds;

    //printf("(%d:%d:%d.%d)\n", hours,minutes,seconds,pictures);

    head[4]=(head[4]&128)|((hours&0xFF)<<2)|((minutes&0x30)<<4);
    head[5]=((minutes&0x0F)<<4)|(head[5]&0x08)|((seconds&0x38)>>3);
    head[6]=((seconds&0x07)<<5)|((pictures&0x3F)>>1);
    head[7]=((pictures&0x1)<<7)|(head[7]&0x7F);
}

/* vi:set ai ts=4 sw=4 expandtab: */
