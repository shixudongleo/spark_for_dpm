/*
#
# Headers for the generic display subsystem
#
# $Id: display.h 347 2009-03-02 23:27:14Z keescook $
#
# Copyright (C) 2004-2009 Kees Cook
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
#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "GOPchop.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "video_out.h"
#include <mpeg2dec/mpeg2.h>

#ifdef __cplusplus
}
#endif

#include <gdk/gdkx.h>

#include "GroupOfPictures.h"

#define GOPCHOP_OUTPUT_LIBVO 0
#define GOPCHOP_OUTPUT_PIPE  1

struct display_engine_t {
    int              opt_show_states;
    mpeg2dec_t      *mpeg2dec;
    const mpeg2_info_t *parser_info;

    int             desired_output;

    // FIXME: these two sections should be a union

    // libvo output
    vo_instance_t   *output;
    GdkWindow       *frame_window;

    // pipe output
    int pipes[2];
    int *client_pipe;


    GroupOfPictures *GOP;

    int              last_valid;    // is the "last" stuff valid?
    uint32_t         last_group;    // last group sent to decoder
    int              last_flushed;  // did the last group get flushed?
};

// display a specific frame from the given gop. frame<0 means last frame
void display_frame(struct display_engine_t * engine,
                   uint32_t group, int frame);

// display each frame of the requested GOP.
// if the "last_group" preceeds this one, and it wasn't "last_flushed",
// then we can safely continue displaying this GOP.
void display_gop  (struct display_entine_t * engine,
                   uint32_t group, int flush_end);

// returns NULL on failure, or an init'd structure that caller must free
struct display_engine_t * display_open(
                   int desired_output,
                   char * opt_output_arg,
                   char * opt_videoout,
                   int opt_show_states);
// closes and unallocated all stuff
void display_close(struct display_engine_t * engine);

// hooks up a group of pictures (use NULL to unhook)
void display_hookup_gop(struct display_engine_t * engine,
                        GroupOfPictures *GOP);

// is this window the display window?
bool display_is_window(struct display_engine_t * engine,
                       GdkWindow * window);

// redraw the display
void display_redraw(struct display_engine_t * engine);

// flush the pictures
void display_flush(struct display_engine_t * engine);

// temporary
void show_GOP(struct display_engine_t * engine, uint32_t wantedGOP);

// extra
vo_open_t * find_libvo_driver(char * desired_driver);

#endif /* _DISPLAY_H_ */
/* vi:set ai ts=4 sw=4 expandtab: */
