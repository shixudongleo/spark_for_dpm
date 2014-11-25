/*
#
# Routines for handling widget madness
#
# $Id: widgets.cpp 347 2009-03-02 23:27:14Z keescook $
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

#include "GOPchop.h"
#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "interface.h"
#include "support.h"

#ifdef __cplusplus
}
#endif

#include "widgets.h"
#include "main.h"

#include <stdio.h>

// windows
GtkWidget *main_window = NULL;
GtkWidget *about_window = NULL;
GtkWidget *error_dialog = NULL;
GtkWidget *GOP_window = NULL;
GtkWidget *preferences_window = NULL;
GtkWidget *new_file_window = NULL;
GtkWidget *open_file_window = NULL;
GtkWidget *save_file_window = NULL;
GtkWidget *export_file_window = NULL;

// the menus and buttons
GtkWidget *menu_load_mpeg2 = NULL;
GtkWidget *menu_load_clip_list = NULL;
GtkWidget *menu_save_clip_list = NULL;
GtkWidget *menu_close = NULL;
GtkWidget *menu_export_mpeg2 = NULL;
GtkWidget *menu_preferences = NULL;
GtkWidget *menu_clear = NULL;
GtkWidget *menu_delete = NULL;
GtkWidget *menu_video_window = NULL;
GtkWidget *button_run = NULL;
GtkWidget *button_refresh = NULL;
GtkWidget *button_prev = NULL;
GtkWidget *button_next = NULL;
GtkWidget *button_start_mark = NULL;
GtkWidget *button_end_mark = NULL;
GtkWidget *main_popup = NULL;
popup_info_t main_popup_info;
gint main_clist_row = -1;
/* FIXME: I should move these into a structure of the global options */
GtkWidget *menu_options_run_loop = NULL;
GtkWidget *menu_options_auto_refresh = NULL;
GtkWidget *menu_options_ignore_errors = NULL;
GtkWidget *menu_options_drop_orphaned_frames = NULL;
GtkWidget *menu_options_adjust_timestamps = NULL;
GtkWidget *menu_options_first_pack = NULL; //TI!

// labels and things
GtkWidget *main_progressbar = NULL;
GtkWidget *main_statusbar = NULL;
GtkWidget *main_clist = NULL;
GtkListStore *main_list_store = NULL;
GtkWidget *main_label_clips = NULL;
GtkWidget *main_label_mark = NULL;
GtkWidget *error_text_why = NULL;
GtkWidget *GOP_selector = NULL;
GtkWidget *GOP_selector_spinbutton = NULL;
GtkWidget *slider_run_speed = NULL;
GtkWidget *GOP_label_filename = NULL;
GtkWidget *GOP_label_GOP = NULL;
GtkWidget *GOP_label_sequence_info = NULL;
GtkWidget *GOP_clist = NULL;
GtkListStore *GOP_list_store = NULL;


/* count each row in a list */
gboolean count_list_rows(GtkTreeModel *model,
                         GtkTreePath *path,
                         GtkTreeIter *iter,
                         gpointer data)
{
    gint * count = (gint *)data;
    if (!count) {
        fprintf(stderr,"NULL counter in %s\n",__FUNCTION__);
        return TRUE; /* stop the traversal */
    }
    count++;
    return FALSE; /* continue the traversal */
}

/* initialize all the widget junk */
void setup_gtk_stuff()
{
    const gchar *mainstatus = "main";
    GtkTreeViewColumn * column;

    GtkTreeIter iter;
    GtkCellRenderer * text_render, * num_render;

    /*
     * FIXME: I really ought to do error checking on each of these
     * GTK_WIDGET lines
     */

    /* figure out where our pixmaps are */
    add_pixmap_directory(PACKAGE_DATA_DIR "/pixmaps/" PACKAGE_NAME);

    /* create our main window */
    main_window = GTK_WIDGET(create_main_window());

    // create our error dialog now
    error_dialog = GTK_WIDGET(create_error_dialog());

    // create our GOP window now
    GOP_window = GTK_WIDGET(create_GOP_window());

    // create our popup
    main_popup = GTK_WIDGET(create_main_popup());
    main_popup_info.x = 0;
    main_popup_info.y = 0;

    // locate menus and buttons
    menu_load_mpeg2 =
        GTK_WIDGET(lookup_widget(GTK_WIDGET(main_window), "menu_load_mpeg2"));
    menu_load_clip_list =
        GTK_WIDGET(lookup_widget(GTK_WIDGET(main_window), "menu_load_clip_list"));
    menu_close =
        GTK_WIDGET(lookup_widget(GTK_WIDGET(main_window), "menu_file_close"));
    menu_save_clip_list =
        GTK_WIDGET(lookup_widget(GTK_WIDGET(main_window), "menu_save_clip_list"));
    menu_export_mpeg2 =
        GTK_WIDGET(lookup_widget(GTK_WIDGET(main_window), "menu_export_mpeg2"));
    menu_preferences =
        GTK_WIDGET(lookup_widget(GTK_WIDGET(main_window), "menu_options_preferences"));

    menu_clear =
        GTK_WIDGET(lookup_widget(GTK_WIDGET(main_window), "menu_file_clear"));
    menu_delete =
        GTK_WIDGET(lookup_widget(GTK_WIDGET(main_window), "menu_delete"));

    menu_video_window =
        GTK_WIDGET(lookup_widget(GTK_WIDGET(main_window), "menu_video_window"));

    button_run =
        GTK_WIDGET(lookup_widget(GTK_WIDGET(main_window), "button_run"));
    button_refresh =
        GTK_WIDGET(lookup_widget(GTK_WIDGET(main_window), "button_refresh"));
    button_prev =
        GTK_WIDGET(lookup_widget(GTK_WIDGET(main_window), "button_prev"));
    button_next =
        GTK_WIDGET(lookup_widget(GTK_WIDGET(main_window), "button_next"));
    button_start_mark =
        GTK_WIDGET(lookup_widget
                   (GTK_WIDGET(main_window), "button_start_mark"));
    button_end_mark =
        GTK_WIDGET(lookup_widget(GTK_WIDGET(main_window), "button_end_mark"));

    // locate things
    main_progressbar =
        GTK_WIDGET(lookup_widget
                   (GTK_WIDGET(main_window), "main_progressbar"));
    main_statusbar =
        GTK_WIDGET(lookup_widget(GTK_WIDGET(main_window), "main_statusbar"));
    GOP_selector =
        GTK_WIDGET(lookup_widget(GTK_WIDGET(main_window), "GOP_selector"));
    GOP_selector_spinbutton =
        GTK_WIDGET(lookup_widget(GTK_WIDGET(main_window), "GOP_selector_spinbutton"));
    slider_run_speed =
        GTK_WIDGET(lookup_widget(GTK_WIDGET(main_window), "slider_run_speed"));
    main_label_clips =
        GTK_WIDGET(lookup_widget(GTK_WIDGET(main_window), "main_label_clips"));
    main_label_mark =
        GTK_WIDGET(lookup_widget(GTK_WIDGET(main_window), "main_label_mark"));
    main_clist =
        GTK_WIDGET(lookup_widget(GTK_WIDGET(main_window), "main_clist"));
    main_clist_row = -1;        // nothing selected
    GOP_clist =
        GTK_WIDGET(lookup_widget(GTK_WIDGET(GOP_window), "GOP_clist"));
    GOP_label_filename =
        GTK_WIDGET(lookup_widget
                   (GTK_WIDGET(GOP_window), "GOP_label_filename"));
    GOP_label_GOP =
        GTK_WIDGET(lookup_widget(GTK_WIDGET(GOP_window), "GOP_label_GOP"));
    GOP_label_sequence_info =
        GTK_WIDGET(lookup_widget(GTK_WIDGET(GOP_window), "GOP_label_sequence_info"));
    error_text_why =
        GTK_WIDGET(lookup_widget(GTK_WIDGET(error_dialog), "error_text_why"));

    ////////////////////
    // do things that glade doesn't handle
    ///////////////////

    gtk_widget_set_sensitive( menu_preferences, TRUE );
    gtk_widget_set_sensitive( menu_export_mpeg2, FALSE );
    gtk_widget_set_sensitive( menu_video_window, FALSE );
    gtk_widget_set_sensitive( menu_clear, FALSE );
    gtk_widget_set_sensitive( menu_delete, FALSE );

    // list renderers
    text_render = gtk_cell_renderer_text_new();
    num_render  = gtk_cell_renderer_text_new();
    g_object_set(num_render,"xalign",1.0,NULL);

    // create our main list data store
    main_list_store = gtk_list_store_new(N_MAIN_ITEMS,
                                     G_TYPE_STRING, G_TYPE_UINT, G_TYPE_UINT);
    gtk_tree_view_set_model(GTK_TREE_VIEW(main_clist),
                            GTK_TREE_MODEL(main_list_store));

    /* set up drag/drop within the widget */
    g_object_set(G_OBJECT(main_clist),"reorderable",TRUE,NULL);

    /* just an example line to see something */
    /*
    gtk_list_store_append (main_list_store, &iter);
    gtk_list_store_set (main_list_store, &iter,
        ITEM_FILENAME, "/asdf/asdf/asdf/hi.mpg",
        ITEM_GOP_FIRST,  2,
        ITEM_GOP_LAST,  52,
        -1);
    */

    column = gtk_tree_view_column_new_with_attributes(
                "Filename", text_render,
                "text", ITEM_FILENAME,
                NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW(main_clist),
                                 GTK_TREE_VIEW_COLUMN(column));

    column = gtk_tree_view_column_new_with_attributes(
                "First GOP", num_render,
                "text", ITEM_GOP_FIRST,
                NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW(main_clist),
                                 GTK_TREE_VIEW_COLUMN(column));
    gtk_tree_view_column_set_alignment(column, 1.0);

    column = gtk_tree_view_column_new_with_attributes(
                "Last GOP", num_render,
                "text", ITEM_GOP_LAST,
                NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW(main_clist),
                                 GTK_TREE_VIEW_COLUMN(column));
    gtk_tree_view_column_set_alignment(column, 1.0);

    // create our GOP list data store
    GOP_list_store = gtk_list_store_new(N_GOP_ITEMS,
                                        G_TYPE_STRING,
                                        G_TYPE_STRING,
                                        G_TYPE_STRING);
    gtk_tree_view_set_model(GTK_TREE_VIEW(GOP_clist),
                            GTK_TREE_MODEL(GOP_list_store));

    column = gtk_tree_view_column_new_with_attributes(
                "Item", text_render,
                "text", ITEM_DATATYPE,
                NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW(GOP_clist),
                                 GTK_TREE_VIEW_COLUMN(column));

    column = gtk_tree_view_column_new_with_attributes(
                "Parent Offset", num_render,
                "text", ITEM_OFFSET,
                NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW(GOP_clist),
                                 GTK_TREE_VIEW_COLUMN(column));
    gtk_tree_view_column_set_alignment(column, 1.0);

    column = gtk_tree_view_column_new_with_attributes(
                "Data Bytes", num_render,
                "text", ITEM_SIZE,
                NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW(GOP_clist),
                                 GTK_TREE_VIEW_COLUMN(column));
    gtk_tree_view_column_set_alignment(column, 1.0);

    // set up ranges
    gtk_range_set_range(GTK_RANGE(GOP_selector), 0.0, 0.1);
    gtk_spin_button_set_range(GTK_SPIN_BUTTON(GOP_selector_spinbutton), 0.0, 0.1);

    // word wrap
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(error_text_why), GTK_WRAP_WORD);

    // further initialization
    status_on(mainstatus, _("Ready"));

    /* display our main window and GOP information window */
    gtk_widget_show(main_window);
    gtk_widget_show(GOP_window);
}

/* vi:set ai ts=4 sw=4 expandtab: */
