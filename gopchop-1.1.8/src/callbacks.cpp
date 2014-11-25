/* vi:set ai ts=4 sw=4 expandtab: */
/*
#
# C++-ized callbacks.c file produced by Glade for GTK event callbacks
#
# $Id: callbacks.cpp 347 2009-03-02 23:27:14Z keescook $
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

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include <stdio.h>

extern "C"
{
#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "video_out.h"
}

#include "main.h"
#include "widgets.h"
#include "gettext.h"

#include "Picture.h"
#include "sessions.h"

#include "display.h"

extern struct display_engine_t * engine;
extern int desired_output;
extern char * opt_videoout;
extern int opt_show_states;
extern char * opt_pipe;

static void
slider_updated_actions(uint32_t GOP)
{
    // only do something if we have output, and auto refreshing
    if (options.auto_refresh)
        show_GOP(engine,GOP);

    if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button_run))) {
        display_flush(engine);
    }

    // make sure the parser is loaded
    if (mpeg2parser->getParsingState()==PARSER_STATE_FINISHED)
        update_GOP_info(GOP);

    // handle start/end button availability
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button_start_mark))) {
        gtk_widget_set_sensitive(button_end_mark, (GOP >= mark_start));
    }
}

void file_is_loaded()
{
    List *GOPs;
    List *pictures;
    gchar buf[128];
    char *pathname;
    int picture_count = 0;

    // toggle our file options
    gtk_widget_set_sensitive(menu_load_mpeg2, FALSE);
    gtk_widget_set_sensitive(menu_load_clip_list, FALSE);
    gtk_widget_set_sensitive(menu_save_clip_list, TRUE);
    gtk_widget_set_sensitive(menu_export_mpeg2, TRUE);
    gtk_widget_set_sensitive(menu_close, TRUE);

    gtk_widget_set_sensitive(menu_delete, TRUE);
    gtk_widget_set_sensitive(menu_clear, TRUE);

    gtk_widget_set_sensitive(menu_video_window, TRUE);

    gtk_widget_set_sensitive(button_run, TRUE);
    gtk_widget_set_sensitive(button_prev, TRUE);
    gtk_widget_set_sensitive(button_next, TRUE);
    gtk_widget_set_sensitive(button_refresh, TRUE);
    gtk_widget_set_sensitive(button_start_mark, TRUE);
    gtk_widget_set_sensitive(button_end_mark, FALSE);

    // adjust the slider scales
    gtk_range_set_range(GTK_RANGE(GOP_selector), 0.0, (gdouble) (mpeg2parser->numGOPs()));
    gtk_spin_button_set_range(GTK_SPIN_BUTTON(GOP_selector_spinbutton), 0.0, (gdouble) (mpeg2parser->numGOPs()-1));

    // start decode window
    display_close(engine);
    engine = display_open(desired_output,opt_pipe,opt_videoout,opt_show_states);
    set_GOP_selected(0);
    flush();
    slider_updated_actions(0);

    pathname = mpeg2parser->getFilename();
    if (!pathname)
        pathname = "unknown";
    gtk_label_set_text(GTK_LABEL(GOP_label_filename), pathname);
}

void file_is_unloaded()
{
    // stop decode window
    display_close(engine);
    engine = NULL;
    set_GOP_selected(0);

    // toggle our file options
    gtk_widget_set_sensitive(menu_load_mpeg2, TRUE);
    gtk_widget_set_sensitive(menu_load_clip_list, TRUE);
    gtk_widget_set_sensitive(menu_save_clip_list, FALSE);
    gtk_widget_set_sensitive(menu_export_mpeg2, FALSE);
    gtk_widget_set_sensitive(menu_close, FALSE);

    gtk_widget_set_sensitive(menu_delete, FALSE);
    gtk_widget_set_sensitive(menu_clear, FALSE);

    gtk_widget_set_sensitive(menu_video_window, FALSE);

    gtk_widget_set_sensitive(button_run, FALSE);
    gtk_widget_set_sensitive(button_prev, FALSE);
    gtk_widget_set_sensitive(button_next, FALSE);
    gtk_widget_set_sensitive(button_refresh, FALSE);
    gtk_widget_set_sensitive(button_start_mark, FALSE);
    gtk_widget_set_sensitive(button_end_mark, FALSE);

    // adjust the slider scales
    gtk_range_set_range(GTK_RANGE(GOP_selector), 0.0, 0.1);
    gtk_spin_button_set_range(GTK_SPIN_BUTTON(GOP_selector_spinbutton), 0.0, 0.1);

    // wipe out GOP info window
    clear_GOP_info();

    // wipe out clip window
    clear_GOP_slices();

    // reset parser
    mpeg2parser->reset();

    if (error_dialog)
    {
        // hide the errors (no errors now!)
        gtk_widget_hide(error_dialog);
    }
}


/*
 * #############################################################################
 * ######################## Start Menu Related Functions #######################
 * #############################################################################
 */

/* +-----------------------------------+
 * | File Menu Related Functions       |
 * +-----------------------------------+ */
void
on_menu_load_mpeg2_activate            (GtkMenuItem    *menuitem,
                                        gpointer        user_data)
{
    if( !new_file_window ) {
        new_file_window = create_new_file_window();
    }
    gtk_widget_show( new_file_window );
}


void
on_menu_load_clip_list_activate        (GtkMenuItem    *menuitem,
                                        gpointer        user_data)
{
    if (!open_file_window)
    {
        open_file_window = create_open_file_window();
    }
    gtk_widget_show(open_file_window);
}


void
on_menu_save_clip_list_activate        (GtkMenuItem    *menuitem,
                                        gpointer        user_data)
{
    if (!save_file_window)
    {
        save_file_window = create_save_file_window();
    }
    gtk_widget_show(save_file_window);
}


void
on_menu_export_mpeg2_activate          (GtkMenuItem    *menuitem,
                                        gpointer        user_data)
{
    if (!export_file_window)
    {
        export_file_window = create_export_file_window();
    }
    gtk_widget_show(export_file_window);
}


void
on_menu_file_close_activate            (GtkMenuItem    *menuitem,
                                        gpointer        user_data)
{
    file_is_unloaded();
}


void
on_menu_file_quit_activate             (GtkMenuItem    *menuitem,
                                        gpointer        user_data)
{
    mpeg2parser->reset();
    gtk_main_quit();
}


/* +-----------------------------------+
 * | Edit Menu Related Functions       |
 * +-----------------------------------+ */
void
on_menu_delete_activate                (GtkMenuItem    *menuitem,
                                        gpointer        user_data)
{
    GtkTreeSelection *selection;
    GtkTreeIter       iter;

    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(main_clist));
    if (!gtk_tree_selection_get_selected(selection, NULL, &iter)) {
        return; /* nothing selected: skip */
    }

    remove_GOP_slice(&iter);
}


void
on_menu_file_clear_activate            (GtkMenuItem    *menuitem,
                                        gpointer        user_data)
{
    clear_GOP_slices();
}

static char *
string_merge(char * one, char * two)
{
    char * result=NULL;

    if (!(result=(char*)malloc(strlen(one)+strlen(two)+1))) {
        perror("malloc");
        exit(1);
    }
    sprintf(result,"%s%s",one,two);
    return result;
}


// are we _displaying_ preferences or saving them?
static void
handle_simple_preferences(gboolean display)
{
    rc_parse_item * parse_item;

    for (parse_item = parsable_items;
         parse_item->option;
         parse_item++) {

//        printf("%s '%s' ...\n",display ? "Populating" : "Storing", parse_item->option);

        // skip missing variables
        if (!parse_item->variable) continue;

        switch (parse_item->vartype) {
            case RC_TYPE_BOOLEAN:
            {
                gboolean * value = (gboolean*)parse_item->variable;
                GtkWidget * button;
                char *name = string_merge(parse_item->option,"_checkbutton");

                button = lookup_widget( GTK_WIDGET(preferences_window), name );
                if (display) {
                    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(button),
                                                  (*value == false) ?
                                                    FALSE : TRUE );
                }
                else {
                    *value = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(button) );
                }

                if (name) free(name);
                break;
            }
            case RC_TYPE_INTEGER:
            {
                int * value = (int*)parse_item->variable;
                GtkWidget *spinner, *range;
                char *spin_name  = string_merge(parse_item->option,"_spinner");
                char *slide_name = string_merge(parse_item->option,"_slider");

                spinner = lookup_widget(GTK_WIDGET(preferences_window),
                                        spin_name );
                range = lookup_widget(GTK_WIDGET(preferences_window),
                                      slide_name);
                if (display) {
                    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinner),
                                              (gdouble)*value);
                    gtk_range_set_value(GTK_RANGE(range),
                                        (gdouble)*value);
                }
                else {
                    *value = (int) gtk_range_get_value(GTK_RANGE(range));
                }


                if (spin_name) free(spin_name);
                if (slide_name) free(slide_name);
                break;
            }
            case RC_TYPE_STRING:
            {
                // string combo boxes are not simple...
                break;
            }
            default:
            {
                printf("Unknown parseable vartype: %s:%d\n",
                       parse_item->option,parse_item->vartype);
                break;
            }
        }
    }
}


void
on_menu_options_preferences_activate   (GtkMenuItem    *menuitem,
                                        gpointer        user_data)
{
    GtkToggleButton *button;
    GtkCombo        *combo;
    GtkRange        *range;
    GtkSpinButton   *spinner;
    GList           *items;
    int i;
    vo_driver_t const *drivers;
    
    items = NULL;

    if( !preferences_window ) {
        preferences_window = create_preferences_window();
    }

        /* set the combo box up with the correct values. */
        combo = GTK_COMBO(lookup_widget(GTK_WIDGET(preferences_window), "video_output_driver_combo" ));
        gchar * vidpref = NULL;
        drivers = vo_drivers();
        for( i = 0; drivers[i].name; i++ ) {
            items = g_list_append(items, drivers[i].name);

            // while looking through the list of available video drivers,
            // the prefered driver is xv2, followed by xv, and finally x11.
            // If the user hasn't selected one, this one will be used.
            if ( !strcmp(drivers[i].name,"xv2") ||
                 (!vidpref && !strcmp(drivers[i].name,"xv"))) {
                if (vidpref) free(vidpref);
                vidpref=g_strdup(drivers[i].name);
            }
        }
        if (!vidpref) vidpref=g_strdup("x11");

        gtk_combo_set_popdown_strings(combo, items);

        if( !options.video_driver_ptr ) options.video_driver_ptr = vidpref;
        else                            if (vidpref) free(vidpref);

        gtk_entry_set_text(GTK_ENTRY(combo->entry),
                           (const gchar*)options.video_driver_ptr);

    handle_simple_preferences(true);

    gtk_widget_show(preferences_window);
}


/* +-----------------------------------+
 * | View Menu Related Functions       |
 * +-----------------------------------+ */
void
on_menu_info_window_activate           (GtkMenuItem    *menuitem,
                                        gpointer        user_data)
{
    if( TRUE == gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) ) {
        if( NULL == GOP_window ) {
            GOP_window = GTK_WIDGET(create_GOP_window());
        }

        gtk_widget_show(GOP_window);
    } else {
        gtk_widget_hide(GOP_window);
    }
}


void
on_menu_video_window_activate          (GtkMenuItem    *menuitem,
                                        gpointer        user_data)
{

}

/* +-----------------------------------+
 * | Help Menu Related Functions       |
 * +-----------------------------------+ */
void
on_menu_help_about_activate            (GtkMenuItem    *menuitem,
                                        gpointer        user_data)
{
    static char buf[1024];
    GtkWidget *label;

    if (!about_window)
    {
        about_window = create_about_window();

        // set version in window
        snprintf(buf, 1023, _("GOPchop %s (%s)"), VERSION, __DATE__);
        label = GTK_WIDGET(lookup_widget(GTK_WIDGET(about_window),
                                         "about_label_version"));
        gtk_label_set_text(GTK_LABEL(label), buf);

        label = GTK_WIDGET(lookup_widget(GTK_WIDGET(about_window),
                                         "about_label_cvsid"));
        gtk_label_set_text(GTK_LABEL(label), GOPchop_cvsid);
    }

    gtk_widget_show(about_window);
}
/*
 * #############################################################################
 * ######################## End Menu Related Functions #########################
 * #############################################################################
 */


/*
 * #############################################################################
 * ######################## Start FileSelection Related Functions ##############
 * #############################################################################
 */

/* +-----------------------------------+
 * | Preferences Related Functions     |
 * +-----------------------------------+ */
void
on_preferences_okbutton_clicked        (GtkButton       *button,
                                        gpointer         user_data)
{
    const gchar     *string;
    GtkCombo        *combo;

    // handle video mode selection
    combo = GTK_COMBO(lookup_widget(GTK_WIDGET(preferences_window), "video_output_driver_combo" ));
    string = gtk_entry_get_text(GTK_ENTRY(combo->entry));
    options.video_driver_ptr = options.video_driver;
    g_snprintf(options.video_driver_ptr, 24, "%s", string);
    opt_videoout = options.video_driver_ptr;

    // load simple prefs from window
    handle_simple_preferences(false);
    
    gtk_widget_hide(preferences_window);

    // preference setting side-effects
    options.run_speed = options.default_run_speed;
    gtk_range_set_value(GTK_RANGE(slider_run_speed), options.run_speed);
    mpeg2parser->set_ignore_endcode(options.ignore_endcode);

    rc_save(PACKAGE,parsable_items);
}

void
on_preferences_cancelbutton_clicked    (GtkButton       *button,
                                        gpointer         user_data)
{
    gtk_widget_hide(preferences_window);
}

gboolean
on_preferences_window_delete_event     (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
    gtk_widget_hide(preferences_window);
    return TRUE;
}

void
on_preferences_run_speed_slider_value_changed
                                        (GtkRange        *range,
                                        gpointer         user_data)
{
    gdouble value;
    GtkSpinButton *spinner;
    
    value = gtk_range_get_value(range);
    spinner = GTK_SPIN_BUTTON(lookup_widget(GTK_WIDGET(preferences_window), "default_run_speed_spinner" ));
    gtk_spin_button_set_value(spinner, value);
}

void
on_preferences_run_speed_spinner_value_changed
                                        (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
    gdouble value;
    GtkRange *range;

    value = gtk_spin_button_get_value(spinbutton);
    range = GTK_RANGE(lookup_widget(GTK_WIDGET(preferences_window), "default_run_speed_slider"));
    gtk_range_set_value( range, value );
}


/* +-----------------------------------+
 * | New Related Functions             |
 * +-----------------------------------+ */
void
on_new_file_okbutton_clicked           (GtkButton       *button,
                                        gpointer         user_data)
{
    char *filename;

    gtk_widget_hide(new_file_window);
    flush();

    filename = (char*)
        gtk_file_selection_get_filename(GTK_FILE_SELECTION
                                        (new_file_window));

    open_file(filename);
}

void
on_new_file_cancelbutton_clicked       (GtkButton       *button,
                                        gpointer         user_data)
{
    gtk_widget_hide(new_file_window);
}

void
on_new_file_window_destroy             (GtkObject       *object,
                                        gpointer         user_data)
{
    new_file_window = NULL;
}

gboolean
on_new_file_window_delete_event        (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
    gtk_widget_hide(new_file_window);
    return TRUE;
}


/* +-----------------------------------+
 * | Open Related Functions            |
 * +-----------------------------------+ */
void
on_open_file_okbutton_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
    char *filename;
    int   load_status;

    gtk_widget_hide(open_file_window);
    flush();

    filename = (char*)
        gtk_file_selection_get_filename(GTK_FILE_SELECTION
                                        (open_file_window));

    load_status = load_session(filename);
    if( SESSION_LOADED_OK == load_status ) {
    } else {
        show_error( error_to_string(load_status) );
    }
}

void
on_open_file_cancelbutton_clicked      (GtkButton       *button,
                                        gpointer         user_data)
{
    gtk_widget_hide(open_file_window);
}

void
on_open_file_window_destroy            (GtkObject       *object,
                                        gpointer         user_data)
{
    open_file_window = NULL;
}

gboolean
on_open_file_window_delete_event       (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
    gtk_widget_hide(open_file_window);
    return TRUE;
}


/* +-----------------------------------+
 * | Save Related Functions            |
 * +-----------------------------------+ */
void
on_save_file_okbutton_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
    gchar     *filename;
    GtkWidget *dialog;

    gint result = GTK_RESPONSE_OK;

    filename = (char*)
        gtk_file_selection_get_filename(GTK_FILE_SELECTION
                                        (save_file_window));
    if( !access(filename, F_OK) ) {
        dialog = gtk_message_dialog_new(GTK_WINDOW(save_file_window),
                                        GTK_DIALOG_DESTROY_WITH_PARENT,
                                        GTK_MESSAGE_QUESTION,
                                        GTK_BUTTONS_OK_CANCEL,
                                        "File '%s' exists.  Overwrite?",
                                        filename);
        result = gtk_dialog_run( GTK_DIALOG(dialog) );
        gtk_widget_destroy( dialog );
    }

    gtk_widget_hide(save_file_window);
    flush();

    if( GTK_RESPONSE_OK == result ) {
        save_session( filename );
    }
}

void
on_save_file_cancelbutton_clicked      (GtkButton       *button,
                                        gpointer         user_data)
{
    gtk_widget_hide(save_file_window);
}

gboolean
on_save_file_window_delete_event       (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
    gtk_widget_hide(save_file_window);
    return TRUE;
}


/* +-----------------------------------+
 * | Export Related Functions          |
 * +-----------------------------------+ */
void
on_export_file_okbutton_clicked        (GtkButton       *button,
                                        gpointer         user_data)
{
    gchar *filename;
    GtkWidget *dialog;

    gint result = GTK_RESPONSE_OK;

    filename = (char*)
        gtk_file_selection_get_filename(GTK_FILE_SELECTION
                                        (export_file_window));

    if( !access(filename, F_OK) ) {
        dialog = gtk_message_dialog_new(GTK_WINDOW(save_file_window),
                                        GTK_DIALOG_DESTROY_WITH_PARENT,
                                        GTK_MESSAGE_QUESTION,
                                        GTK_BUTTONS_OK_CANCEL,
                                        "File '%s' exists.  Overwrite?",
                                        filename);
        result = gtk_dialog_run( GTK_DIALOG(dialog) );
        gtk_widget_destroy( dialog );
    }

    gtk_widget_hide(export_file_window);
    flush();

    if( GTK_RESPONSE_OK == result ) {
        save_file(filename);
    }

}


void
on_export_file_cancelbutton_clicked    (GtkButton       *button,
                                        gpointer         user_data)
{
    gtk_widget_hide(export_file_window);
}

void
on_export_file_window_destroy          (GtkObject       *object,
                                        gpointer         user_data)
{
    export_file_window = NULL;
}

gboolean
on_export_file_window_delete_event     (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
    gtk_widget_hide(export_file_window);
    return TRUE;
}
/*
 * #############################################################################
 * ######################## End FileSelection Related Functions ################
 * #############################################################################
 */

/*
 * #############################################################################
 * ######################## Start Main Window Related Functions ################
 * #############################################################################
 */
void
on_GOP_selector_value_changed          (GtkRange        *range,
                                        gpointer         user_data)
{
    uint32_t GOP;

    GOP = (uint32_t) gtk_range_get_value(range);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(GOP_selector_spinbutton), (gdouble)GOP);

    slider_updated_actions(GOP);
}

void
on_GOP_selector_spinbutton_value_changed
                                        (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
    uint32_t GOP;

    GOP = (uint32_t) gtk_spin_button_get_value(spinbutton);
    gtk_range_set_value(GTK_RANGE(GOP_selector), (gdouble)GOP);

    slider_updated_actions(GOP);
}

void
on_slider_run_speed_value_changed      (GtkRange        *range,
                                        gpointer         user_data)
{
    options.run_speed = (int) gtk_range_get_value(range);
}
/*
 * #############################################################################
 * ######################## End Main Window Related Functions ##################
 * #############################################################################
 */


static gboolean run_saved_refresh = true;
void on_button_run_toggled(GtkToggleButton * togglebutton, gpointer user_data)
{
    uint32_t GOPs, i;

    GOPs = mpeg2parser->numGOPs();
    i = get_GOP_selected();

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(togglebutton))) {
        // if you clicked "Run", you want to see it...
        run_saved_refresh = options.auto_refresh;
        options.auto_refresh = true;
    }

    while (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(togglebutton)))
    {
        set_GOP_selected(i);
        flush();
        i = get_GOP_selected();
        i+=options.run_speed;
        if (i >= GOPs)
        {
            if (options.run_loop)
                i = 0;
            else
            {
                // flip back off now
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(togglebutton),
                                             FALSE);
                flush();
            }
        }
    }
    flush();
    display_flush(engine);

    options.auto_refresh = run_saved_refresh;
}


void on_about_window_destroy(GtkObject * object, gpointer user_data)
{
    about_window = NULL;
}


void on_about_button_cool_clicked(GtkButton * button, gpointer user_data)
{
    gtk_widget_hide(about_window);
    gtk_widget_destroy(about_window);
}


void on_error_button_annoying_clicked(GtkButton * button, gpointer user_data)
{
    gtk_widget_hide(error_dialog);
}

void
on_button_start_mark_toggled           (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(togglebutton))) {
        /* start mark */
        char buf[128];
        mark_start = get_GOP_selected();

        gtk_widget_set_sensitive(button_end_mark, TRUE);

        sprintf(buf, "%d", mark_start);
        gtk_label_set_text(GTK_LABEL(main_label_mark), buf);
    }
    else {
        /* undo start mark */
        gtk_widget_set_sensitive(button_end_mark, FALSE);
        gtk_label_set_text(GTK_LABEL(main_label_mark), "no mark set");
    }
}

void on_button_end_mark_clicked(GtkButton * button, gpointer user_data)
{
    mark_end = get_GOP_selected();

    if (mark_end >= mark_start)
    {
        gtk_widget_set_sensitive(button_end_mark, FALSE);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button_start_mark),
                                     FALSE);

        gtk_label_set_text(GTK_LABEL(main_label_mark), "no mark set");

        add_GOP_slice(mark_start, mark_end);
    }
    else
    {
        // FIXME: do something to "flash" the startmark label
    }
}


void on_button_refresh_clicked(GtkButton * button, gpointer user_data)
{
    show_GOP(engine,get_GOP_selected());
    display_redraw(engine);
}


gboolean
on_main_clist_key_press_event(GtkWidget * widget,
                              GdkEventKey * event, gpointer user_data)
{
    GtkTreeSelection *selection;
    GtkTreeIter       iter;

    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
    if (!gtk_tree_selection_get_selected(selection, NULL, &iter)) {
        return FALSE; /* nothing selected: skip */
    }

    switch (event->keyval)
    {
    case GDK_Delete:
    case GDK_KP_Delete:
    case GDK_BackSpace:
        remove_GOP_slice(&iter);
        return TRUE;
    }
    return FALSE;
}


gboolean
on_main_clist_button_press_event(GtkWidget * widget,
                                 GdkEventButton * event, gpointer user_data)
{

#if 0
    if (event->button == 3)     // right-click
    {
        main_popup_info.x = event->x;
        main_popup_info.y = event->y;
        gtk_menu_popup(GTK_MENU(main_popup), NULL, NULL, NULL,
                       &main_popup_info, event->button, event->time);
        return TRUE;
    }
#endif

    return FALSE;
}


void on_main_popup_delete_activate(GtkMenuItem * menuitem, gpointer user_data)
{
    gint row, col;
    popup_info_t *info;

    fprintf( stderr, "on_main_popup_delete_activate\n" );

    if (info = (popup_info_t *) user_data)
    {
        gtk_clist_get_selection_info(GTK_CLIST(main_clist),
                                     info->x, info->y, &row, &col);

        //printf("bloa! (x %d y %d is row %d col %d)\n");
    }
    else
        printf("%s",
               _("Oops.  NULL info in on_main_popup_delete_activate\n"));

}


void on_save_file_window_destroy(GtkObject * object, gpointer user_data)
{
    save_file_window = NULL;
}


void on_button_next_clicked(GtkButton * button, gpointer user_data)
{
    set_GOP_selected(get_GOP_selected() + 1);
}


void on_button_prev_clicked(GtkButton * button, gpointer user_data)
{
    uint32_t gop;

    // don't wrap to the end
    gop=get_GOP_selected();
    if (gop>0) {
        gop--;
    }
    set_GOP_selected(gop);
}


#if 0
void
on_menu_options_ignore_errors_activate(GtkMenuItem * menuitem,
                                       gpointer user_data)
{
    options.ignore_errors=GTK_CHECK_MENU_ITEM(menuitem)->active;
    rc_save(PACKAGE,parsable_items);
}


void
on_menu_options_drop_orphaned_frames_activate
                                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    options.drop_orphaned_frames=GTK_CHECK_MENU_ITEM(menuitem)->active;
    rc_save(PACKAGE,parsable_items);
}



void
on_menu_options_adjust_timestamps_activate
                                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    options.adjust_timestamps=GTK_CHECK_MENU_ITEM(menuitem)->active;
    rc_save(PACKAGE,parsable_items);
}
#endif

gboolean
on_main_window_delete_event            (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
    mpeg2parser->reset();
    gtk_main_quit();

    return FALSE;
}


gboolean
on_GOP_window_delete_event             (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
    GtkWidget *menuitem;

    menuitem = GTK_WIDGET(lookup_widget(GTK_WIDGET(main_window), "menu_info_window"));
    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(menuitem), FALSE );
    gtk_widget_hide(GOP_window);
    return TRUE;
}
/******************************************************************************/
