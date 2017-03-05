/* desktop session
 * Copyright (C) 2005 Benjamin Otte <otte@gnome.org>
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

#include <glib/gi18n.h>

#include "byzanzsession.h"

static int duration = 10;
static int delay = 1;
static gboolean cursor = FALSE;
static gboolean audio = FALSE;
static gboolean verbose = FALSE;
static char *exec = NULL;
static cairo_rectangle_int_t area = { 0, 0, G_MAXINT / 2, G_MAXINT / 2 };

static GOptionEntry entries[] = 
{
  { "duration", 'd', 0, G_OPTION_ARG_INT, &duration, N_("Duration of animation (default: 10 seconds)"), N_("SECS") },
  { "exec", 'e', 0, G_OPTION_ARG_STRING, &exec, N_("Command to execute and time"), N_("COMMAND") },
  { "delay", 0, 0, G_OPTION_ARG_INT, &delay, N_("Delay before start (default: 1 second)"), N_("SECS") },
  { "cursor", 'c', 0, G_OPTION_ARG_NONE, &cursor, N_("Record mouse cursor"), NULL },
  { "audio", 'a', 0, G_OPTION_ARG_NONE, &audio, N_("Record audio"), NULL },
  { "x", 'x', 0, G_OPTION_ARG_INT, &area.x, N_("X coordinate of rectangle to record"), N_("PIXEL") },
  { "y", 'y', 0, G_OPTION_ARG_INT, &area.y, N_("Y coordinate of rectangle to record"), N_("PIXEL") },
  { "width", 'w', 0, G_OPTION_ARG_INT, &area.width, N_("Width of recording rectangle"), N_("PIXEL") },
  { "height", 'h', 0, G_OPTION_ARG_INT, &area.height, N_("Height of recording rectangle"), N_("PIXEL") },
  { "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, N_("Be verbose"), NULL },
  { NULL }
};

static void
verbose_print (const gchar *format, ...) G_GNUC_PRINTF (1, 2);
static void
verbose_print (const gchar *format, ...)
{
  gchar *buffer;
  va_list args;

  if (!verbose)
    return;
  
  va_start (args, format);
  buffer = g_strdup_vprintf (format, args);
  va_end (args);
  g_print ("%s", buffer);
}

static void
usage (void)
{
  g_print (_("usage: %s [OPTIONS] filename\n"), g_get_prgname ());
  g_print (_("       %s --help\n"), g_get_prgname ());
}

static void
session_notify_cb (ByzanzSession *session, GParamSpec *pspec, gpointer unused)
{
  const GError *error = byzanz_session_get_error (session);
  
  if (g_str_equal (pspec->name, "error")) {
    g_print (_("Error during recording: %s\n"), error->message);
    gtk_main_quit ();
    return;
  }

  if (!byzanz_session_is_encoding (session)) {
    verbose_print (_("Recording done.\n"));
    gtk_main_quit ();
  }
}

static gboolean
stop_recording (gpointer session)
{
  verbose_print (_("Recording completed. Finishing encoding...\n"));
  byzanz_session_stop (session);
  
  return FALSE;
}

static void
child_exited (GPid     pid,
              gint     status,
              gpointer session)
{
  if (status != 0)
    verbose_print (_("Child exited with error %d\n"), status);

  stop_recording (session);
}

static gboolean
start_recording (gpointer session)
{
  if (exec) {
    GError *error = NULL;
    GPid pid;
    char **args;

    if (!g_shell_parse_argv (exec, NULL, &args, &error)) {
      g_print (_("Invalid exec command: %s\n"), error->message);
      g_error_free (error);
      gtk_main_quit ();
      return FALSE;
    }
  
    if (!g_spawn_async (NULL, args, NULL, G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD,
        NULL, NULL, &pid, &error)) {
      g_print (_("Failed to execute: %s\n"), error->message);
      g_error_free (error);
      gtk_main_quit ();
      return FALSE;
    }

    verbose_print (_("Recording starts. Will record until child exits...\n"));
    g_child_watch_add (pid, child_exited, session);
  } else {
    verbose_print (_("Recording starts. Will record %d seconds...\n"), duration / 1000);
    g_timeout_add (duration, stop_recording, session);
  }
  byzanz_session_start (session);
  
  return FALSE;
}

static gboolean
clamp_to_window (cairo_rectangle_int_t *out, GdkWindow *window, cairo_rectangle_int_t *in)
{
  cairo_rectangle_int_t window_area = { 0, };

  window_area.width = gdk_window_get_width (window);
  window_area.height = gdk_window_get_height (window);
  return gdk_rectangle_intersect ((GdkRectangle *) in,
                                  (GdkRectangle *) &window_area,
                                  (GdkRectangle *) in);
}

int
main (int argc, char **argv)
{
  ByzanzSession *rec;
  GOptionContext* context;
  GError *error = NULL;
  GFile *file;
  
  g_set_prgname (argv[0]);
#ifdef GETTEXT_PACKAGE
  bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
#endif

  context = g_option_context_new (_("record your current desktop session"));
#ifdef GETTEXT_PACKAGE
  g_option_context_set_translation_domain(context, GETTEXT_PACKAGE);
#endif

  g_option_context_add_main_entries (context, entries, GETTEXT_PACKAGE);
  g_option_context_add_group (context, gtk_get_option_group (TRUE));
  if (!g_option_context_parse (context, &argc, &argv, &error)) {
    g_print (_("Wrong option: %s\n"), error->message);
    usage ();
    return 1;
  }
  if (argc != 2) {
    usage ();
    return 0;
  }
  if (!clamp_to_window (&area, gdk_get_default_root_window (), &area)) {
    g_print (_("Given area is not inside desktop.\n"));
    return 1;
  }
  file = g_file_new_for_commandline_arg (argv[1]);
  rec = byzanz_session_new (file, byzanz_encoder_get_type_from_file (file),
      gdk_get_default_root_window (), &area, cursor, audio);
  g_object_unref (file);
  g_signal_connect (rec, "notify", G_CALLBACK (session_notify_cb), NULL);
  
  delay = MAX (delay, 1);
  delay = (delay - 1) * 1000;
  duration = MAX (duration, 0);
  duration *= 1000;
  g_timeout_add (delay, start_recording, rec);
  
  gtk_main ();

  g_object_unref (rec);
  return 0;
}
