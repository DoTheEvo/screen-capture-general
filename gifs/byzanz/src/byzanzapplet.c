/* desktop session
 * Copyright (C) 2005,2009 Benjamin Otte <otte@gnome.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <unistd.h>

#include <panel-applet.h>
#include <gio/gio.h>
#include <glib/gstdio.h>
#include "paneltogglebutton.h"
#include <glib/gi18n.h>

#include "byzanzencoder.h"
#include "byzanzselect.h"
#include "byzanzsession.h"

static GQuark index_quark = 0;

typedef struct {
  PanelApplet *		applet;		/* the applet we manage */

  GtkWidget *		button;		/* recording button */
  GtkWidget *		image;		/* image displayed in button */
  GtkWidget *           dialog;         /* file chooser */
  GFile *               file;           /* file selected for recording */
  GType                 encoder_type;   /* last selected encoder type or 0 if none */
  
  ByzanzSession *	rec;		/* the session (if recording) */

  /* config */
  GSettings *           settings;       /* the settings for this applet */
  int			method;		/* recording method that was set */
} AppletPrivate;
#define APPLET_IS_RECORDING(applet) ((applet)->tmp_file != NULL)

/*** PENDING RECORDING ***/

static void
byzanz_applet_show_error (AppletPrivate *priv, const char *error, const char *details, ...) G_GNUC_PRINTF (3, 4);
static void
byzanz_applet_show_error (AppletPrivate *priv, const char *error, const char *details, ...)
{
  GtkWidget *dialog, *parent;
  gchar *msg;
  va_list args;

  g_return_if_fail (error != NULL);
  
  if (details) {
    va_start (args, details);
    msg = g_strdup_vprintf (details, args);
    va_end (args);
  } else {
    msg = NULL;
  }
  if (priv)
    parent = gtk_widget_get_toplevel (GTK_WIDGET (priv->applet));
  else
    parent = NULL;
  dialog = gtk_message_dialog_new (GTK_WINDOW (parent), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR,
      GTK_BUTTONS_CLOSE, "%s", error);
  if (msg)
    gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s", msg);
  g_free (msg);
  g_signal_connect (dialog, "response", G_CALLBACK (gtk_widget_destroy), NULL);
  gtk_widget_show_all (dialog);
}

/*** APPLET ***/

typedef enum {
  BYZANZ_APPLET_IDLE,
  BYZANZ_APPLET_SELECT_AREA,
  BYZANZ_APPLET_RECORDING,
  BYZANZ_APPLET_ENCODING,
  /* add more here */
  BYZANZ_APPLET_N_STATES
} ByzanzAppletState;

static struct {
  const char *  tooltip;                /* tooltip to present on the applet */
  const char *  stock_icon;             /* icon to use for this state */
  gboolean      active;                 /* wether the togglebutton should be active */
} state_info[BYZANZ_APPLET_N_STATES] = {
  [BYZANZ_APPLET_IDLE] = { N_("Record your desktop"), GTK_STOCK_MEDIA_RECORD, FALSE },
  [BYZANZ_APPLET_SELECT_AREA] = { N_("Select area to record"), GTK_STOCK_CANCEL, TRUE },
  [BYZANZ_APPLET_RECORDING] = { N_("End current recording"), GTK_STOCK_MEDIA_STOP, TRUE },
  [BYZANZ_APPLET_ENCODING] = { N_("Abort encoding of recording"), GTK_STOCK_STOP, TRUE }
};

static ByzanzAppletState
byzanz_applet_compute_state (AppletPrivate *priv)
{
  if (priv->rec == NULL) {
    if (priv->file)
      return BYZANZ_APPLET_SELECT_AREA;
    else
      return BYZANZ_APPLET_IDLE;
  } else {
    if (byzanz_session_is_recording (priv->rec))
      return BYZANZ_APPLET_RECORDING;
    else
      return BYZANZ_APPLET_ENCODING;
  }
}

static void button_clicked_cb (GtkToggleButton *button, AppletPrivate *priv);
static gboolean
byzanz_applet_update (gpointer data)
{
  AppletPrivate *priv = data;
  ByzanzAppletState state;

  state = byzanz_applet_compute_state (priv);

  g_signal_handlers_block_by_func (priv->button, button_clicked_cb, priv);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->button), state_info[state].active);
  g_signal_handlers_unblock_by_func (priv->button, button_clicked_cb, priv);

  gtk_image_set_from_icon_name (GTK_IMAGE (priv->image), 
      state_info[state].stock_icon, GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_widget_set_tooltip_text (priv->button, _(state_info[state].tooltip));
  
  return TRUE;
}

static void
byzanz_applet_set_default_method (AppletPrivate *priv, int id)
{
  if (priv->method == id)
    return;
  if (id >= (gint) byzanz_select_get_method_count ())
    return;

  priv->method = id;

  g_settings_set_string (priv->settings, "method", byzanz_select_method_get_name (id));
}

static void
byzanz_applet_session_notify (AppletPrivate *priv)
{
  const GError *error;

  g_assert (priv->rec != NULL);

  error = byzanz_session_get_error (priv->rec);
  if (error) {
    byzanz_applet_show_error (priv, error->message, NULL);
    g_signal_handlers_disconnect_by_func (priv->rec, byzanz_applet_session_notify, priv);
    g_object_unref (priv->rec);
    priv->rec = NULL;
  } else if (!byzanz_session_is_encoding (priv->rec)) {
    g_signal_handlers_disconnect_by_func (priv->rec, byzanz_applet_session_notify, priv);
    g_object_unref (priv->rec);
    priv->rec = NULL;
  }

  byzanz_applet_update (priv);
}

static int method_response_codes[] = { GTK_RESPONSE_ACCEPT, GTK_RESPONSE_APPLY, GTK_RESPONSE_OK, GTK_RESPONSE_YES };

static void
byzanz_applet_select_done (GdkWindow *window, const cairo_rectangle_int_t *area, gpointer data)
{
  AppletPrivate *priv = data;

  if (window != NULL) {
    GType encoder_type = priv->encoder_type;

    if (encoder_type == 0)
      encoder_type = byzanz_encoder_get_type_from_file (priv->file);
    priv->rec = byzanz_session_new (priv->file, encoder_type, window, area, FALSE,
        g_settings_get_boolean (priv->settings, "record-audio"));
    g_signal_connect_swapped (priv->rec, "notify", G_CALLBACK (byzanz_applet_session_notify), priv);
    byzanz_session_start (priv->rec);
  }
  g_object_unref (priv->file);
  priv->file = NULL;

  if (priv->rec)
    byzanz_applet_session_notify (priv);
  else
    byzanz_applet_update (priv);
}

static void
panel_applet_start_response (GtkWidget *dialog, int response, AppletPrivate *priv)
{
  guint i;
  GtkFileFilter *filter;

  priv->file = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (dialog));
  if (priv->file == NULL)
    goto out;

  for (i = 0; i < byzanz_select_get_method_count (); i++) {
    if (response == method_response_codes[i]) {
      char *uri = g_file_get_uri (priv->file);
      g_settings_set_string (priv->settings, "save-filename", uri);
      g_free (uri);
      byzanz_applet_set_default_method (priv, i);
      break;
    }
  }
  if (i >= byzanz_select_get_method_count ())
    goto out;

  filter = gtk_file_chooser_get_filter (GTK_FILE_CHOOSER (priv->dialog));
  if (filter && gtk_file_filter_get_needed (filter) != 0) {
    priv->encoder_type = byzanz_encoder_get_type_from_filter (filter);
  } else {
    /* It's the "All files" filter */
    priv->encoder_type = 0;
  }

  g_settings_set_boolean (priv->settings, "record-audio",
      gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (
          gtk_file_chooser_get_extra_widget (GTK_FILE_CHOOSER (priv->dialog)))));

  gtk_widget_destroy (dialog);
  priv->dialog = NULL;
  byzanz_select_method_select (priv->method, byzanz_applet_select_done, priv);
  byzanz_applet_update (priv);
  return;

out:
  gtk_widget_destroy (dialog);
  priv->dialog = NULL;
  if (priv->file) {
    g_object_unref (priv->file);
    priv->file = NULL;
  }
  byzanz_applet_update (priv);
}

static void
byzanz_applet_start_recording (AppletPrivate *priv)
{
  if (priv->dialog == NULL) {
    char *uri;
    guint i;
    GType type;
    ByzanzEncoderIter iter;
    GtkFileFilter *filter;

    priv->dialog = gtk_file_chooser_dialog_new (_("Record your desktop"),
        GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (priv->applet))),
        GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
        NULL);
    g_assert (G_N_ELEMENTS (method_response_codes) >= byzanz_select_get_method_count ());
    for (i = 0; i < byzanz_select_get_method_count (); i++) {
      gtk_dialog_add_button (GTK_DIALOG (priv->dialog),
          byzanz_select_method_get_mnemonic (i), method_response_codes[i]);
    }
    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (filter, _("All files"));
    gtk_file_filter_add_custom (filter, 0, (GtkFileFilterFunc) gtk_true, NULL, NULL);
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (priv->dialog), filter);
    for (type = byzanz_encoder_type_iter_init (&iter);
         type != G_TYPE_NONE;
         type = byzanz_encoder_type_iter_next (&iter)) {
      filter = byzanz_encoder_type_get_filter (type);
      if (filter) {
        gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (priv->dialog), filter);
        if (priv->encoder_type && priv->encoder_type == byzanz_encoder_get_type_from_filter (filter))
          gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (priv->dialog), filter);
        g_object_unref (filter);
      }
    }

    gtk_file_chooser_set_extra_widget (GTK_FILE_CHOOSER (priv->dialog),
        gtk_check_button_new_with_label (_("Record audio")));
    if (g_settings_get_boolean (priv->settings, "record-audio")) {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (
          gtk_file_chooser_get_extra_widget (GTK_FILE_CHOOSER (priv->dialog))), TRUE);
    }

    gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER (priv->dialog), FALSE);
    uri = g_settings_get_string (priv->settings, "save-filename");
    if (!uri || uri[0] == '\0' ||
        !gtk_file_chooser_set_uri (GTK_FILE_CHOOSER (priv->dialog), uri)) {
      gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (priv->dialog), g_get_home_dir ());
    }
    g_free (uri);
    gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (priv->dialog), TRUE);
    g_signal_connect (priv->dialog, "response", G_CALLBACK (panel_applet_start_response), priv);

    gtk_widget_show_all (priv->dialog);

    /* need to show before setting the default response, otherwise the filechooser reshuffles it */
    gtk_dialog_set_default_response (GTK_DIALOG (priv->dialog), method_response_codes[priv->method]);
  }
  gtk_window_present (GTK_WINDOW (priv->dialog));

  byzanz_applet_update (priv);
}

static void
button_clicked_cb (GtkToggleButton *button, AppletPrivate *priv)
{
  ByzanzAppletState state = byzanz_applet_compute_state (priv);

  switch (state) {
    case BYZANZ_APPLET_IDLE:
      byzanz_applet_start_recording (priv);
      break;
    case BYZANZ_APPLET_SELECT_AREA:
      /* FIXME: allow cancelling here */
      break;
    case BYZANZ_APPLET_RECORDING:
      byzanz_session_stop (priv->rec);
      break;
    case BYZANZ_APPLET_ENCODING:
      byzanz_session_abort (priv->rec);
      break;
    case BYZANZ_APPLET_N_STATES:
    default:
      g_assert_not_reached ();
      break;
  }
}

static void
destroy_applet (GtkWidget *widget, AppletPrivate *priv)
{
  if (priv->rec)
    g_object_unref (priv->rec);
  g_free (priv);
}

static void 
byzanz_about_cb (GtkAction *action, AppletPrivate *priv)
{
  const gchar *authors[] = {
    "Benjamin Otte <otte@gnome.org>", 
    NULL
   };

  gtk_show_about_dialog( NULL,
    "name",                _("Desktop Session"), 
    "version",             VERSION,
    "copyright",           "\xC2\xA9 2005-2006 Benjamin Otte",
    "comments",            _("Record what's happening on your desktop"),
    "authors",             authors,
    "translator-credits",  _("translator-credits"),
    NULL );
}

static const GtkActionEntry byzanz_menu_actions [] = {
  { "ByzanzAbout", GTK_STOCK_ABOUT, N_("_About"),
    NULL, NULL,
    G_CALLBACK (byzanz_about_cb) }
};

static gboolean
byzanz_applet_fill (PanelApplet *applet, const gchar *iid, gpointer data)
{
  AppletPrivate *priv;
  GtkActionGroup *action_group;
  char *ui_path;
  char *method;
  
  if (!index_quark)
    index_quark = g_quark_from_static_string ("Byzanz-Index");
#ifdef GETTEXT_PACKAGE
  bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
#endif

  gtk_window_set_default_icon_name ("byzanz-record-desktop");

  priv = g_new0 (AppletPrivate, 1);
  priv->applet = applet;
  priv->settings = panel_applet_settings_new (applet, "org.gnome.byzanz.applet");

  g_signal_connect (applet, "destroy", G_CALLBACK (destroy_applet), priv);
  panel_applet_add_preferences (applet, "/schemas/apps/byzanz-applet/prefs",
      NULL);
  panel_applet_set_flags (applet, PANEL_APPLET_EXPAND_MINOR);
  action_group = gtk_action_group_new ("Byzanz Applet Actions");
#ifdef GETTEXT_PACKAGE
  gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
#endif
  gtk_action_group_add_actions (action_group,
				byzanz_menu_actions,
				G_N_ELEMENTS (byzanz_menu_actions),
				priv);
  ui_path = g_build_filename (BYZANZ_MENU_UI_DIR, "byzanzapplet.xml", NULL);
  panel_applet_setup_menu_from_file (PANEL_APPLET (applet),
				     ui_path, action_group);
  g_free (ui_path);
  g_object_unref (action_group);

  method = g_settings_get_string (priv->settings, "method");
  priv->method = byzanz_select_method_lookup (method);
  g_free (method);
  if (priv->method < 0)
    priv->method = 0;

  priv->button = panel_toggle_button_new ();
  priv->image = gtk_image_new ();
  gtk_container_add (GTK_CONTAINER (priv->button), priv->image);
  g_signal_connect (priv->button, "toggled", G_CALLBACK (button_clicked_cb), priv);
  gtk_container_add (GTK_CONTAINER (priv->applet), priv->button);
  panel_applet_set_background_widget (applet, priv->button);

  byzanz_applet_update (priv);
  gtk_widget_show_all (GTK_WIDGET (applet));
  
  return TRUE;
}

PANEL_APPLET_OUT_PROCESS_FACTORY ("ByzanzAppletFactory",
				  PANEL_TYPE_APPLET,
				  byzanz_applet_fill,
				  NULL)


