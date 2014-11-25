/*
#
# This is to handle GTK initialization and utility
#
# $Id: widgets.h 347 2009-03-02 23:27:14Z keescook $
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

#ifndef _WIDGETS_H_
#define _WIDGETS_H_

#include "config.h"

#include <gtk/gtk.h>
#include <glib.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef struct
{
    gint x;
    gint y;
}
popup_info_t;

// windows
extern GtkWidget *main_window;
extern GtkWidget *about_window;
extern GtkWidget *error_dialog;
extern GtkWidget *GOP_window;
extern GtkWidget *preferences_window;
extern GtkWidget *new_file_window;
extern GtkWidget *open_file_window;
extern GtkWidget *save_file_window;
extern GtkWidget *export_file_window;

// menus and buttons
extern GtkWidget *menu_load_mpeg2;
extern GtkWidget *menu_load_clip_list;
extern GtkWidget *menu_save_clip_list;
extern GtkWidget *menu_close;
extern GtkWidget *menu_export_mpeg2;
extern GtkWidget *menu_preferences;
extern GtkWidget *menu_clear;
extern GtkWidget *menu_delete;
extern GtkWidget *menu_video_window;
extern GtkWidget *button_run;
extern GtkWidget *button_refresh;
extern GtkWidget *button_prev;
extern GtkWidget *button_next;
extern GtkWidget *button_start_mark;
extern GtkWidget *button_end_mark;
extern GtkWidget *main_popup;
extern popup_info_t main_popup_info;

// labels, lists, and things
extern GtkWidget *main_progressbar;
extern GtkWidget *main_statusbar;
extern GtkWidget *main_clist;
extern GtkListStore *main_list_store;
extern GtkWidget *main_label_clips;
extern GtkWidget *main_label_mark;
extern GtkWidget *GOP_selector;
extern GtkWidget *GOP_selector_spinbutton;
extern GtkWidget *GOP_clist;
extern GtkListStore *GOP_list_store;
extern GtkWidget *GOP_label_filename;
extern GtkWidget *GOP_label_GOP;
extern GtkWidget *GOP_label_sequence_info;
extern GtkWidget *slider_run_speed;
extern GtkWidget *overwrite_label_filename;
extern GtkWidget *error_text_why;


// type info for gtk widgets
/* main_clist contents */
enum {          
    ITEM_FILENAME,  /* Filename the clip came from */
    ITEM_GOP_FIRST, /* First GOP to start the clip from */
    ITEM_GOP_LAST,  /* GOP to end the clip with */
    N_MAIN_ITEMS    /* Max Item number for initializing GtkTreeView */
};
/* GOP_clist contents */
enum {
    ITEM_DATATYPE,  /* What kind of frame the info is detailing */
    ITEM_OFFSET,    /* Where in the stream the frame starts */
    ITEM_SIZE,      /* How large the frame is in bytes */
    N_GOP_ITEMS     /* Max Item number for initializing GtkTreeView */
};

/* widget functions */
void setup_gtk_stuff();
gboolean count_list_rows(GtkTreeModel *model,
                         GtkTreePath *path,
                         GtkTreeIter *iter,
                         gpointer data);


#ifdef __cplusplus
}
#endif

#endif /* _WIDGETS_H_ */

/* vi:set ai ts=4 sw=4 expandtab: */
