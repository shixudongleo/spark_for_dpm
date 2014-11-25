/* vi:set ai ts=4 sw=4 expandtab: */
#include <gtk/gtk.h>


/* File Menu Prototypes */
void
on_menu_load_mpeg2_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
void
on_menu_load_clip_list_activate        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
void
on_menu_save_clip_list_activate        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
void
on_menu_export_mpeg2_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
void
on_menu_file_close_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
void
on_menu_file_quit_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
/* Edit Menu Prototypes */
void
on_menu_delete_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
void
on_menu_file_clear_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
void
on_menu_options_preferences_activate   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
/* View Menu Prototypes */
void
on_menu_info_window_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
void
on_menu_video_window_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
/* Help Menu Prototypes */
void
on_menu_help_about_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
/* Preferences Related Functions */
void
on_preferences_okbutton_clicked        (GtkButton       *button,
                                        gpointer         user_data);
void
on_preferences_cancelbutton_clicked    (GtkButton       *button,
                                        gpointer         user_data);
gboolean
on_preferences_window_delete_event     (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);
void
on_preferences_run_speed_slider_value_changed
                                        (GtkRange        *range,
                                        gpointer         user_data);
void
on_preferences_run_speed_spinner_value_changed
                                        (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);
/* New File Selction Related Functions */
void
on_new_file_okbutton_clicked           (GtkButton       *button,
                                        gpointer         user_data);
void
on_new_file_cancelbutton_clicked       (GtkButton       *button,
                                        gpointer         user_data);
void
on_new_file_window_destroy             (GtkObject       *object,
                                        gpointer         user_data);
gboolean
on_new_file_window_delete_event        (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);
/* Open File Selction Related Functions */
void
on_open_file_okbutton_clicked          (GtkButton       *button,
                                        gpointer         user_data);
void
on_open_file_cancelbutton_clicked      (GtkButton       *button,
                                        gpointer         user_data);
void
on_open_file_window_destroy            (GtkObject       *object,
                                        gpointer         user_data);
gboolean
on_open_file_window_delete_event       (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);
/* Save File Selction Related Functions */
void
on_save_file_okbutton_clicked          (GtkButton       *button,
                                        gpointer         user_data);
void
on_save_file_cancelbutton_clicked      (GtkButton       *button,
                                        gpointer         user_data);
void
on_save_file_window_destroy            (GtkObject       *object,
                                        gpointer         user_data);
gboolean
on_save_file_window_delete_event       (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);
/* Export File Selction Related Functions */
void
on_export_file_okbutton_clicked        (GtkButton       *button,
                                        gpointer         user_data);
void
on_export_file_cancelbutton_clicked    (GtkButton       *button,
                                        gpointer         user_data);
void
on_export_file_window_destroy          (GtkObject       *object,
                                        gpointer         user_data);
gboolean
on_export_file_window_delete_event     (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);
void
on_slider_run_speed_value_changed      (GtkRange        *range,
                                        gpointer         user_data);
void
on_GOP_selector_value_changed          (GtkRange        *range,
                                        gpointer         user_data);
void
on_GOP_selector_spinbutton_value_changed
                                       (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void on_button_prev_clicked(GtkButton * button, gpointer user_data);
void
on_button_run_toggled(GtkToggleButton * togglebutton, gpointer user_data);
void on_button_next_clicked(GtkButton * button, gpointer user_data);
void on_about_window_destroy(GtkObject * object, gpointer user_data);
void on_about_button_cool_clicked(GtkButton * button, gpointer user_data);
void
on_fileselection_button_ok_clicked(GtkButton * button, gpointer user_data);
void
on_fileselection_button_cancel_clicked(GtkButton * button,
                                       gpointer user_data);
void on_fileselection_window_destroy(GtkObject * object, gpointer user_data);
void on_error_button_annoying_clicked(GtkButton * button, gpointer user_data);
void on_button_end_mark_clicked(GtkButton * button, gpointer user_data);
void on_button_refresh_clicked(GtkButton * button, gpointer user_data);
void on_button_start_mark_clicked(GtkButton * button, gpointer user_data);
gboolean
on_main_clist_key_press_event(GtkWidget * widget,
                              GdkEventKey * event, gpointer user_data);
gboolean
on_main_clist_button_press_event(GtkWidget * widget,
                                 GdkEventButton * event, gpointer user_data);
void
on_main_popup_delete_activate(GtkMenuItem * menuitem, gpointer user_data);
void
on_main_clist_unselect_row(GtkCList * clist,
                           gint row,
                           gint column, GdkEvent * event, gpointer user_data);
void
on_main_clist_select_row(GtkCList * clist,
                         gint row,
                         gint column, GdkEvent * event, gpointer user_data);
void on_saveselection_window_destroy(GtkObject * object, gpointer user_data);
void on_overwrite_button_ok_clicked(GtkButton * button, gpointer user_data);
void
on_overwrite_button_cancel_clicked(GtkButton * button, gpointer user_data);
void
on_menu_options_ignore_errors_activate(GtkMenuItem * menuitem,
                                       gpointer user_data);
void
on_menu_options_drop_orphaned_frames_activate
                                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
void
on_button_start_mark_toggled           (GtkToggleButton *togglebutton,
                                        gpointer         user_data);
void
on_menu_options_adjust_timestamps_activate
                                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
gboolean
on_main_window_delete_event            (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);
gboolean
on_GOP_window_delete_event             (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);
/******************************************************************************/
