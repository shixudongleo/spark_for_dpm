/*
#
# Display subsystem.
#
# $Id: display.cpp 347 2009-03-02 23:27:14Z keescook $
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

#include "display.h"

#ifdef __cplusplus
extern "C" {
#endif

/* libmpeg2 */
#include "video_out.h"
#include <mpeg2dec/mpeg2.h>

#ifdef __cplusplus
}
#endif

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "GOPchop.h"
#include "main.h"
#include "List.h"
#include "Pack.h"
#include "ElementStream.h"
#include "mpeg2consts.h"

static
void frame_draw(struct display_engine_t * engine)
{
    if (!engine) return;

    if (engine && engine->output && engine->output->draw &&
        engine->parser_info && engine->parser_info->display_fbuf) {

        engine->output->draw(engine->output,
                     engine->parser_info->display_fbuf->buf,
                     engine->parser_info->display_fbuf->id);
    }
}

// decode function
static
void decode(struct display_engine_t * engine, uint8_t * buffer, size_t len)
{
    Window xwindow;
    vo_setup_result_t setup_result;
    int state;
#ifdef DEBUG_VES
    FILE *ves_debug;
#endif

    //printf("decoding %ld bytes of VES data\n",len);

    /* make sure output and decoder are available */
    if (!engine) {
        fprintf(stderr, "%s", _("Display engine missing!\n"));
        return;
    }
    if (!engine->output) {
        fprintf(stderr, "%s", _("Output window missing!\n"));
        return;
    }
    if (!engine->mpeg2dec) {
        fprintf(stderr, "%s", _("MPEG2 decoder missing!\n"));
        return;
    }

    if (!buffer) {
        fprintf(stderr,"%s",_("decode called with a missing buffer!?\n"));
        return;
    }

    //if (len) {
        mpeg2_buffer(engine->mpeg2dec, buffer, buffer + len);
        if (!(engine->parser_info = mpeg2_info(engine->mpeg2dec)))
        {
            fprintf(stderr, "%s", _("MPEG2 decoder info not available!?\n"));
            exit(1);
        }
    //}

#ifdef DEBUG_VES
    /* dump the VES packets to a file */
    ves_debug = fopen("video.es", "a");
    fwrite(buffer, len, 1, ves_debug);
    fclose(ves_debug);
#endif

    while ((state = mpeg2_parse(engine->mpeg2dec)) != STATE_BUFFER)
    {
        switch (state)
        {
            case STATE_SEQUENCE:
                if (engine->opt_show_states) printf("STATE_SEQUENCE\n");
                if (!engine->parser_info->sequence) {
                    fprintf(stderr,"%s",_("Got STATE_SEQUENCE without a sequence!?\n"));
                    break;
                }
                if (engine->opt_show_states)
                    printf("saw sequence (%ux%u, pix %u:%u, disp %ux%u)\n",
                        engine->parser_info->sequence->width,
                        engine->parser_info->sequence->height,
                        engine->parser_info->sequence->pixel_width,
                        engine->parser_info->sequence->pixel_height,
                        engine->parser_info->sequence->display_width*engine->parser_info->sequence->pixel_width/engine->parser_info->sequence->pixel_height,
                        engine->parser_info->sequence->picture_height
                        );
                /* sequence started, so initialize output window */
                if (engine->output->setup(engine->output,
                                  engine->parser_info->sequence->width,
                                  engine->parser_info->sequence->height,
                                  engine->parser_info->sequence->display_width*engine->parser_info->sequence->pixel_width/engine->parser_info->sequence->pixel_height,
                                  engine->parser_info->sequence->height,
                                  engine->parser_info->sequence->chroma_width,
                                  engine->parser_info->sequence->chroma_height,
                                  &setup_result))
                {
                    fprintf(stderr, "%s", _("Failed to setup display\n"));
                    return;
                }

                if (engine->output->window_handle) {
                    xwindow = (Window)engine->output->window_handle(engine->output);
                    engine->frame_window = gdk_window_foreign_new((int)xwindow);
                    if (engine->frame_window) {
                        gdk_window_set_events(engine->frame_window,GDK_ALL_EVENTS_MASK);
                        // this doesn't catch screen motion :(
                        XSelectInput(GDK_DISPLAY(), xwindow,
                                     VisibilityChangeMask
                                     // ExposureMask
                                     // | PointerMotionMask
                                     );
                    }
                }

                /* does the output need to be converted? */
                if (setup_result.convert)
                    mpeg2_convert(engine->mpeg2dec, setup_result.convert, NULL);
                /* can the output provide an fbuf area? */
                if (engine->output->set_fbuf)
                {
                    uint8_t *buf[3];
                    void *id;

                    mpeg2_custom_fbuf(engine->mpeg2dec, 1);

                    engine->output->set_fbuf(engine->output, buf, &id);
                    mpeg2_set_buf(engine->mpeg2dec, buf, id);

                    engine->output->set_fbuf(engine->output, buf, &id);
                    mpeg2_set_buf(engine->mpeg2dec, buf, id);
                }
                else if (engine->output->setup_fbuf)
                {
                    uint8_t *buf[3];
                    void *id;

                    engine->output->setup_fbuf(engine->output, buf, &id);
                    mpeg2_set_buf(engine->mpeg2dec, buf, id);
                    engine->output->setup_fbuf(engine->output, buf, &id);
                    mpeg2_set_buf(engine->mpeg2dec, buf, id);
                    engine->output->setup_fbuf(engine->output, buf, &id);
                    mpeg2_set_buf(engine->mpeg2dec, buf, id);
                }
                break;
            case STATE_PICTURE:
                if (engine->opt_show_states) {
                    printf("STATE_PICTURE:   ");
                    if (engine->parser_info &&
                        engine->parser_info->current_picture) {
                        switch (engine->parser_info->current_picture->flags &
                                PIC_MASK_CODING_TYPE) {
                            case PIC_FLAG_CODING_TYPE_I:
                                printf("I");
                                break;
                            case PIC_FLAG_CODING_TYPE_B:
                                printf("B");
                                break;
                            case PIC_FLAG_CODING_TYPE_P:
                                printf("P");
                                break;
                            case PIC_FLAG_CODING_TYPE_D:
                                printf("D");
                                break;
                            default:
                                printf("unknown");
                                break;
                        }
                        printf("(%02d)",
                               engine->parser_info->current_picture->temporal_reference);
                    }
                    printf("       parsed\n");
                }

                /* picture is starting */
                if (engine->output->set_fbuf)
                {
                    uint8_t *buf[3];
                    void *id;

                    engine->output->set_fbuf(engine->output, buf, &id);
                    mpeg2_set_buf(engine->mpeg2dec, buf, id);
                }
                if (engine->output->start_fbuf) {
                    if (engine->parser_info->current_fbuf) {
                        engine->output->start_fbuf(engine->output,
                                           engine->parser_info->current_fbuf->buf,
                                           engine->parser_info->current_fbuf->id);
                    }
                    else {
                        fprintf(stderr,"%s",_("Got STATE_PICTURE without a current_fbuf!?\n"));
                    }
                }
                break;
            case STATE_SLICE:
            case STATE_END:
#ifdef HAVE_MPEG2_FLUSH_PICTURE
            case STATE_FLUSHED:
#endif
                if (engine->opt_show_states) {
                    printf("STATE_%s       ",
                            state == STATE_SLICE ?    "SLICE:    "
                              : (state == STATE_END ? "END:      "
                                 :                    "FLUSHED:  "));
                    if (engine->parser_info &&
                        engine->parser_info->display_picture)
                    {
                        switch (engine->parser_info->display_picture->flags & PIC_MASK_CODING_TYPE)
                        {
                            case PIC_FLAG_CODING_TYPE_I:
                                printf("I");
                                break;
                            case PIC_FLAG_CODING_TYPE_P:
                                printf("P");
                                break;
                            case PIC_FLAG_CODING_TYPE_B:
                                printf("B");
                                break;
                            case PIC_FLAG_CODING_TYPE_D:
                                printf("D");
                                break;
                            default:
                                printf("unknown");
                                break;
                        }
                        printf("(%02d)",
                           engine->parser_info->display_picture->temporal_reference);
                    }
                    else
                    {
                        printf("      nothing loaded to be");
                    }
                    printf(" rendered\n");
                }

                /* draw current picture */
                frame_draw(engine);
                if (engine->output->discard && engine->parser_info->discard_fbuf)
                    engine->output->discard(engine->output,
                                    engine->parser_info->discard_fbuf->buf,
                                    engine->parser_info->discard_fbuf->id);
                break;
            case STATE_INVALID:
                if (engine->opt_show_states) printf("STATE_INVALID\n");
                break;
            case STATE_INVALID_END:
                if (engine->opt_show_states) printf("STATE_INVALID_END\n");
                break;
            case STATE_SEQUENCE_REPEATED:
                if (engine->opt_show_states) printf("STATE_SEQUENCE_REPEATED\n");
                break;
            case STATE_GOP:
                if (engine->opt_show_states) printf("STATE_GOP\n");
                break;
            case STATE_SLICE_1ST:
                if (engine->opt_show_states) printf("STATE_SLICE_1ST\n");
                break;
            case STATE_PICTURE_2ND:
                if (engine->opt_show_states) printf("STATE_PICTURE_2ND\n");
                break;
            default:
                if (engine->opt_show_states) printf("STATE: %d\n",state);
                break;
        }
    }
    if (engine->opt_show_states) printf("STATE_BUFFER\n");
}

struct display_engine_t * display_open(
        int    desired_output,
        char * opt_output_arg,
        char * opt_videoout,
        int    opt_show_states)
{
    struct display_engine_t * engine;

    if (!(engine=(struct display_engine_t*)calloc(1,sizeof(*engine)))) {
        perror("malloc");
        return NULL;
    }
    engine->opt_show_states=opt_show_states;
    engine->desired_output=desired_output;

    vo_open_t * output_open = NULL;

    if (engine->desired_output==GOPCHOP_OUTPUT_LIBVO)
    {
        // decoding
        uint32_t accel = 0;

        if (!(output_open=find_libvo_driver(opt_videoout)))
        {
            fprintf(stderr, _("Can't find output driver '%s'\n"), opt_videoout);
            exit(1);
        }
    
        if (!(engine->output = output_open()))
        {
            fprintf(stderr, _("Can't open output driver '%s'\n"), opt_videoout);
        }
    
        accel = mpeg2_accel(MPEG2_ACCEL_DETECT);
#ifdef ARCH_X86
        if (accel & MPEG2_ACCEL_X86_MMXEXT)
            printf("%s", _("Using x86 MMXext acceleration\n"));
        else if (accel & MPEG2_ACCEL_X86_3DNOW)
            printf("%s", _("Using x86 3DNow acceleration\n"));
        else if (accel & MPEG2_ACCEL_X86_MMX)
            printf("%s", _("Using x86 MMX acceleration\n"));
        else
#endif
#ifdef ARCH_PPC
        if (accel & MPEG2_ACCEL_PPC_ALTIVEC)
            printf("%s", _("Using PowerPC Altivec acceleration\n"));
        else
#endif
#ifdef ARCH_ALPHA
        if (accel & MPEG2_ACCEL_ALPHA_MVI)
            printf("%s", _("Using Alpha MVI acceleration\n"));
        else if (accel & MPEG2_ACCEL_ALPHA)
            printf("%s", _("Using Alpha acceleration\n"));
        else
#endif
#ifdef ARCH_SPARC
        if (accel & MPEG2_ACCEL_SPARC_VIS)
            printf("%s", _("Using Sparc VIS acceleration\n"));
        else if (accel & MPEG2_ACCEL_SPARC_VIS2)
            printf("%s", _("Using Sparc VIS2 acceleration\n"));
        else
#endif
        printf("%s", _("Using no special acceleration\n"));
    
        if (!(engine->mpeg2dec = mpeg2_init()))
        {
            fprintf(stderr, "%s", _("Cannot initialize mpeg2 decoder!\n"));
            exit(1);
        }
    }
    else if (engine->desired_output == GOPCHOP_OUTPUT_PIPE)
    {
        // try a piped command instead

        if (!opt_output_arg)
        {
            fprintf(stderr, "%s", _("Pipe command required.  Try --pipe\n"));
            exit(1);
        }

        // fork off a command
        if (pipe(engine->pipes) < 0)
        {
            perror("pipe");
            exit(1);
        }

        pid_t pid = fork();

        // failure
        if (pid < 0)
        {
            perror("fork");
            exit(1);
        }

        // child
        if (pid == 0)
        {
            char * shell;

            close(engine->pipes[1]);
            if (dup2(engine->pipes[0], STDIN_FILENO) < 0)
            {
                perror("dup2");
                exit(1);
            }
    
            // get the shell to use
            if (!(shell=getenv("SHELL")))
            {
                fprintf(stderr,"%s",
                        _("Cannot figure out from environment what shell to exec.\n"));
                exit(1);
            }

            execl(shell,shell,"-c", opt_output_arg, NULL);
            perror("execl");
            exit(1);
        }
    
        // parent
        engine->client_pipe = &engine->pipes[1];
        close(engine->pipes[0]);
    }
    else {
        fprintf(stderr,_("Unknown output desired!?\n"));
    }


    return engine;
}

void display_close(struct display_engine_t * engine)
{
    if (!engine) return;

    if (engine->output)
    {
        // invalidate these
        engine->frame_window = NULL;
        engine->parser_info  = NULL;

        mpeg2_close(engine->mpeg2dec);
        engine->mpeg2dec = NULL;
        if (engine->output->close)
            engine->output->close(engine->output);
        engine->output = NULL;
    }

    if (engine->client_pipe)
    {
        close(*engine->client_pipe);
        engine->client_pipe = NULL;
    }

    engine->last_valid=0;
    engine->GOP=NULL;
}

vo_open_t * find_libvo_driver(char * desired_driver)
{
    int i;
    vo_driver_t const *drivers;

    drivers = vo_drivers();
    for (i = 0; drivers[i].name; i++)
        if (!desired_driver || !strcmp(drivers[i].name, desired_driver))
            return drivers[i].open;

    return NULL;
}


void show_GOP(struct display_engine_t * engine, uint32_t wantedGOP)
{
    static int initdone = 0;
    List *packets;
    Pack *packet;
    List *GOPs;
    GroupOfPictures *GOP;
    Bounds *bounds;
    ElementStream *ves;
    int start, stop;

    uint8_t *loc;
    size_t len;

    if (!engine) return;

    if (engine->opt_show_states) printf("show GOP: %u\n",wantedGOP);

    // get our GOP list
    if (!(GOPs = mpeg2parser->getGOPs()))
    {
        printf("%s", _("\tNo GOPs\n"));
        return;
    }

    // get our packet list
    if (!(packets = mpeg2parser->getPackets()))
    {
        printf("%s", _("\tNo packets\n"));
        return;
    }

    // get GOP we're after
    if (!(GOP = (GroupOfPictures *) GOPs->vector(wantedGOP)))
    {
        printf(_("\tGOP %d missing\n"), wantedGOP);
        return;
    }

    // handle reset logic
    if (engine->last_valid) {
        if (wantedGOP == engine->last_group) return;
        if (  (engine->last_group+1 != wantedGOP) ||
              (engine->last_flushed)  ) {
            if (engine->opt_show_states) printf("MPEG2 reset\n");
            mpeg2_reset(engine->mpeg2dec,0);
            if (wantedGOP>0 && !GOP->has_sequence_info()) {
                uint32_t num;
                GroupOfPictures *previous=NULL;

                for (num=wantedGOP-1;
                     ;
                     num--)
                {
                    if (!(previous=(GroupOfPictures *) GOPs->vector(num))) {
                        printf(_("\tGOP %d missing\n"), num);
                        return;
                    }
                    if (previous->has_sequence_info()) break;
                    previous=NULL;

                    if (num==0) break;
                }

                // found a prior sequence header
                if (previous) {
                    sequence_header bytes;
                    if (engine->opt_show_states)
                        printf("Injecting sequence from GOP %d\n",num);
                    previous->get_sequence_header(&bytes);
                    decode(engine,(uint8_t*)&bytes,sizeof(bytes));
                }
                else {
                    if (engine->opt_show_states)
                        printf("could not find needed sequence\n");
                }
            }
        }
    }
    engine->last_group = wantedGOP;
    engine->last_flushed = 0;
    engine->last_valid = 1;


    // get our GOP's range of packets
    bounds = GOP->getPacketBounds();
    // remember our start and finish points
    start = bounds->getFirst();
    stop = bounds->getMax();

    for (uint32_t j = start; j < stop; j++)
    {
        if (!(packet = (Pack *) packets->vector(j)))
        {
            printf("%s", _("NULL packet?!\n"));
            exit(1);
        }

        if (engine->desired_output==GOPCHOP_OUTPUT_LIBVO && engine->output)
        {
            uint32_t ves_start, ves_stop;

            ves_start = packet->getVideoFirst();
            ves_stop = packet->getVideoMax();

            for (uint32_t k = ves_start; k < ves_stop; k++)
            {
                if (!(ves =
                      (ElementStream *)mpeg2parser->getVideo()->vector(k)))
                {
                    printf("%s", _("NULL VES?!\n"));
                    exit(1);
                }
    
                len = ves->getLen();
                loc = (uint8_t *)mpeg2parser->bytesAvail(ves->getStart(), len);
    
                /*
                fprintf(stderr,_("\t\tVES: %d @ %llu (%d): 0x%08x\n"),
                        k,ves->getStart(),len,
                        (unsigned int)loc);
                */
    
                if (loc && len)
                    decode(engine, loc, len);
            }
        }
        else if (engine->desired_output==GOPCHOP_OUTPUT_PIPE && engine->client_pipe)
        {
            // send entire packet to pipe

            len = packet->getLen();
            loc = (uint8_t*)mpeg2parser->bytesAvail(packet->getStart(), len);
            if (loc && engine->client_pipe)
            {
                int written;
    
                // stuff things into the pipe
                while (len > 0)
                {
                    if ((written = write(*(engine->client_pipe), loc, len)) < 0)
                    {
                        perror("write");
                        exit(1);
                    }
                    loc += written;
                    len -= written;
                }
            }
        }
        else {
            fprintf(stderr,_("Unknown output desired!?\n"));
        }

    }
}

void display_flush(struct display_engine_t * engine)
{
    if (engine && engine->last_valid && !engine->last_flushed) {
        if (engine->opt_show_states) printf("-- trying to flush pictures\n");
        /* throw in an end-of-stream to make sure libvo kicks out the last frame */
        if (engine->desired_output==GOPCHOP_OUTPUT_LIBVO && engine->output) {
#ifdef HAVE_MPEG2_FLUSH_PICTURE
            mpeg2_flush_picture(mpeg2dec);
            decode(engine, (uint8_t*)"",0);
#else
            decode(engine, (uint8_t*)seq_end_buf,sizeof(seq_end_buf));
#endif
            engine->last_flushed = 1;
        }
        if (engine->opt_show_states) printf("-- done\n");
    }
}

void display_hookup_gop(struct display_engine_t * engine,
                        GroupOfPictures *GOP)
{
    if (!engine) return;

    engine->GOP=GOP;
    engine->last_valid=0;
}

bool display_is_window(struct display_engine_t * engine,
                               GdkWindow * window)
{
    if (engine && engine->frame_window == window) return true;
    return false;
}

void display_redraw(struct display_engine_t * engine)
{
    if (!engine) return;

    /* how the hell do we keep the window visible?  I don't want to have
     * to totally reprocess the GOP every time we get visibility noticiation
     */
    if (engine->last_valid) {
        // "forget" we were already here, and force a redraw
        uint32_t gop = engine->last_group;
        engine->last_group= gop + 1;
        show_GOP(engine,gop);
        flush();
        display_flush(engine);
    }
}

/* vi:set ai ts=4 sw=4 expandtab: */
