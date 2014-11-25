/*
#
# This is to handle GTK initialization and utility
#
# $Id: main.h 347 2009-03-02 23:27:14Z keescook $
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

#ifndef _MAIN_H_
#define _MAIN_H_

#include "config.h"

#include <gtk/gtk.h>
#include <glib.h>

#include "MPEG2Parser.h"
#include "rc.h"

#ifdef __cplusplus
extern "C" {
#endif


// version
extern char *GOPchop_cvsid;

// objects
extern MPEG2Parser *mpeg2parser;

// global status info
extern const gchar *fileops;
extern uint32_t mark_start;
extern uint32_t mark_end;

/* global option control */
typedef struct
{
    gboolean run_loop;              /* when in "Run" mode, wrap to beginning at end */
    int      run_speed;             /* how many GOPs to jump forward during "run" */
    int      default_run_speed;     /* the speed to start with upon load */
    gboolean auto_refresh;          /* redraw the frames after moving the slider */
    gboolean ignore_errors;         /* don't store errors about file loading */
    gboolean drop_orphaned_frames;  /* get rid of starting B frames on an open GOP */
    gboolean adjust_timestamps;     /* adjust the GOP timestamps as we write clips */
    gchar*   video_driver_ptr;
    gchar    video_driver[24];      /* this is the name of the video driver to use */
    gboolean force_system_header;   /* should a system header be prepended */
    gboolean drop_trailing_pack_with_system_header;/* should final pack with a system header be dropped? */
    gboolean ignore_endcode;       /* should parsing continue past an end code? */
} global_options;
extern global_options options;
extern rc_parse_item parsable_items[];

// supporting functions
void get_clips_from_list(GtkTreeModel *model,
                         GtkTreeIter  *iter,
                         struct clip_list_t * clips);
void progress(char *task, float done);
void status_on(const gchar * owner, char *text);
void status_off(const gchar * owner);
void show_error(const gchar *text);
void flush();
uint32_t get_GOP_selected();
void set_GOP_selected(uint32_t num);
const char *frame_type_str(int type);
void clear_GOP_info();
void update_GOP_info(uint32_t num);
void open_file(char *filename);
void save_file(char *filename);

/* functions fork working on the main clip list */
struct save_clip_info_t {
    FILE       *file;
    uint32_t    GOP_previous;
    uint32_t    count;
    uint32_t    total;
    off_t       written_bytes;
    char       *progress_task;
    uint32_t    written_pictures;
};

void add_GOP_slice(uint32_t start, uint32_t end);
void remove_GOP_slice(GtkTreeIter * iter);
void clear_GOP_slices();
// from callback.cpp (FIXME: move functions over)
void file_is_loaded();

// decode functions
/*
void decode_start();
void decode_stop();
void show_GOP(uint32_t wantedGOP);
*/

#ifdef __cplusplus
}
#endif

#endif /* _MAIN_H_ */

/* vi:set ai ts=4 sw=4 expandtab: */
