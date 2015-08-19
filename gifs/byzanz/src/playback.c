/* desktop session recorder
 * Copyright (C) 2009 Benjamin Otte <otte@gnome.org>
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

#include "byzanzencoder.h"
#include "byzanzserialize.h"

static GOptionEntry entries[] = 
{
  { NULL }
};

static void
usage (void)
{
  g_print (_("usage: %s [OPTIONS] INFILE OUTFILE\n"), g_get_prgname ());
  g_print (_("       %s --help\n"), g_get_prgname ());
}

static void
encoder_notify (ByzanzEncoder *encoder, GParamSpec *pspec, GMainLoop *loop)
{
  const GError *error;

  error = byzanz_encoder_get_error (encoder);
  if (error) {
    g_print ("%s\n", error->message);
    g_main_loop_quit (loop);
  } else if (!byzanz_encoder_is_running (encoder)) {
    g_main_loop_quit (loop);
  }
}

int
main (int argc, char **argv)
{
  GOptionContext* context;
  GError *error = NULL;
  GFile *infile;
  GFile *outfile;
  GInputStream *instream;
  GOutputStream *outstream;
  GMainLoop *loop;
  ByzanzEncoder *encoder;
  
  g_set_prgname (argv[0]);
#ifdef GETTEXT_PACKAGE
  bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
#endif

  context = g_option_context_new (_("process a Byzanz debug recording"));
#ifdef GETTEXT_PACKAGE
  g_option_context_set_translation_domain(context, GETTEXT_PACKAGE);
#endif

  g_option_context_add_main_entries (context, entries, GETTEXT_PACKAGE);
  if (!g_option_context_parse (context, &argc, &argv, &error)) {
    g_print (_("Wrong option: %s\n"), error->message);
    usage ();
    g_error_free (error);
    return 1;
  }
  if (argc != 3) {
    usage ();
    return 0;
  }

  infile = g_file_new_for_commandline_arg (argv[1]);
  outfile = g_file_new_for_commandline_arg (argv[2]);
  loop = g_main_loop_new (NULL, FALSE);

  instream = G_INPUT_STREAM (g_file_read (infile, NULL, &error));
  if (instream == NULL) {
    g_print ("%s\n", error->message);
    g_error_free (error);
    return 1;
  }
  outstream = G_OUTPUT_STREAM (g_file_replace (outfile, NULL, 
        FALSE, G_FILE_CREATE_REPLACE_DESTINATION, NULL, &error));
  if (outstream == NULL) {
    g_print ("%s\n", error->message);
    g_error_free (error);
    return 1;
  }
  encoder = byzanz_encoder_new (byzanz_encoder_get_type_from_file (outfile),
      instream, outstream, FALSE, NULL);
  
  g_signal_connect (encoder, "notify", G_CALLBACK (encoder_notify), loop);
  
  g_main_loop_run (loop);

  g_main_loop_unref (loop);
  g_object_unref (encoder);
  g_object_unref (instream);
  g_object_unref (outstream);
  g_object_unref (infile);
  g_object_unref (outfile);

  return 0;
}
