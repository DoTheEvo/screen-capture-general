/* desktop session recorder
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

#include "byzanzsession.h"

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <cairo.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <X11/extensions/Xdamage.h>
#include <X11/extensions/Xfixes.h>

#include "byzanzencoder.h"
#include "byzanzrecorder.h"
#include "byzanzserialize.h"

/*** MAIN FUNCTIONS ***/

enum {
  PROP_0,
  PROP_RECORDING,
  PROP_ENCODING,
  PROP_ERROR,
  PROP_FILE,
  PROP_AREA,
  PROP_WINDOW,
  PROP_AUDIO,
  PROP_ENCODER_TYPE
};

G_DEFINE_TYPE (ByzanzSession, byzanz_session, G_TYPE_OBJECT)

static void
byzanz_session_get_property (GObject *object, guint param_id, GValue *value, 
    GParamSpec * pspec)
{
  ByzanzSession *session = BYZANZ_SESSION (object);

  switch (param_id) {
    case PROP_ERROR:
      g_value_set_pointer (value, session->error);
      break;
    case PROP_RECORDING:
      g_value_set_boolean (value, byzanz_session_is_recording (session));
      break;
    case PROP_ENCODING:
      g_value_set_boolean (value, byzanz_session_is_encoding (session));
      break;
    case PROP_FILE:
      g_value_set_object (value, session->file);
      break;
    case PROP_AREA:
      g_value_set_boxed (value, &session->area);
      break;
    case PROP_WINDOW:
      g_value_set_object (value, session->window);
      break;
    case PROP_AUDIO:
      g_value_set_boolean (value, session->record_audio);
      break;
    case PROP_ENCODER_TYPE:
      g_value_set_gtype (value, session->encoder_type);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
  }
}

static void
byzanz_session_set_property (GObject *object, guint param_id, const GValue *value, 
    GParamSpec * pspec)
{
  ByzanzSession *session = BYZANZ_SESSION (object);

  switch (param_id) {
    case PROP_FILE:
      session->file = g_value_dup_object (value);
      break;
    case PROP_AREA:
      session->area = *(cairo_rectangle_int_t *) g_value_get_boxed (value);
      break;
    case PROP_WINDOW:
      session->window = g_value_dup_object (value);
      break;
    case PROP_AUDIO:
      session->record_audio = g_value_get_boolean (value);
      break;
    case PROP_ENCODER_TYPE:
      session->encoder_type = g_value_get_gtype (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
  }
}

static void
byzanz_session_set_error (ByzanzSession *session, const GError *error)
{
  GObject *object = G_OBJECT (session);

  if (session->error != NULL)
    return;

  session->error = g_error_copy (error);
  g_object_ref (session);
  g_object_freeze_notify (object);
  g_object_notify (object, "error");
  if (byzanz_recorder_get_recording (session->recorder))
    byzanz_session_stop (session);
  g_object_thaw_notify (object);
  g_object_unref (session);
}

static void
byzanz_session_encoder_notify_cb (ByzanzEncoder * encoder,
                                  GParamSpec *    pspec,
                                  ByzanzSession * session)
{
  if (g_str_equal (pspec->name, "running")) {
    g_object_notify (G_OBJECT (session), "encoding");
  } else if (g_str_equal (pspec->name, "error")) {
    const GError *error = byzanz_encoder_get_error (encoder);

    /* Delete the file, it's broken after all. Don't throw errors if it fails though. */
    g_file_delete (session->file, NULL, NULL);

    /* Cancellation is not an error, it's been requested via _abort() */
    if (!g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
      byzanz_session_set_error (session, error);
  }
}

static void
byzanz_session_recorder_notify_cb (ByzanzRecorder * recorder,
                                   GParamSpec *     pspec,
                                   ByzanzSession *  session)
{
  g_object_notify (G_OBJECT (session), "recording");
}

static guint64
byzanz_session_elapsed (ByzanzSession *session, const GTimeVal *tv)
{
  guint elapsed;

  if (session->start_time.tv_sec == 0 && session->start_time.tv_usec == 0) {
    session->start_time = *tv;
    return 0;
  }

  elapsed = tv->tv_sec - session->start_time.tv_sec;
  elapsed *= 1000;
  elapsed += (tv->tv_usec - session->start_time.tv_usec) / 1000;

  return elapsed;
}

static void
byzanz_session_recorder_image_cb (ByzanzRecorder *       recorder,
                                  cairo_surface_t *      surface,
                                  const cairo_region_t * region,
                                  const GTimeVal *       tv,
                                  ByzanzSession *        session)
{
  GOutputStream *stream;
  GError *error = NULL;

  stream = byzanz_queue_get_output_stream (session->queue);
  if (!byzanz_serialize (stream, byzanz_session_elapsed (session, tv), 
          surface, region, session->cancellable, &error)) {
    byzanz_session_set_error (session, error);
    g_error_free (error);
  }
}

static void
byzanz_session_dispose (GObject *object)
{
  ByzanzSession *session = BYZANZ_SESSION (object);

  byzanz_session_abort (session);

  G_OBJECT_CLASS (byzanz_session_parent_class)->dispose (object);
}

static void
byzanz_session_finalize (GObject *object)
{
  ByzanzSession *session = BYZANZ_SESSION (object);

  g_assert (session != NULL);

  g_object_unref (session->recorder);
  if (session->encoder) {
    g_signal_handlers_disconnect_by_func (session->encoder, byzanz_session_encoder_notify_cb, session);
    g_object_unref (session->encoder);
  }
  g_object_unref (session->window);
  g_object_unref (session->file);
  g_object_unref (session->queue);

  if (session->error)
    g_error_free (session->error);

  G_OBJECT_CLASS (byzanz_session_parent_class)->finalize (object);
}

static void
byzanz_session_constructed (GObject *object)
{
  ByzanzSession *session = BYZANZ_SESSION (object);
  GOutputStream *stream;

  session->recorder = byzanz_recorder_new (session->window, &session->area);
  g_signal_connect (session->recorder, "notify::recording", 
      G_CALLBACK (byzanz_session_recorder_notify_cb), session);
  g_signal_connect (session->recorder, "image", 
      G_CALLBACK (byzanz_session_recorder_image_cb), session);

  /* FIXME: make async */
  stream = G_OUTPUT_STREAM (g_file_replace (session->file, NULL, 
        FALSE, G_FILE_CREATE_REPLACE_DESTINATION, session->cancellable, &session->error));
  if (stream != NULL) {
    session->encoder = byzanz_encoder_new (session->encoder_type, 
        byzanz_queue_get_input_stream (session->queue),
        stream, session->record_audio, session->cancellable);
    g_signal_connect (session->encoder, "notify", 
        G_CALLBACK (byzanz_session_encoder_notify_cb), session);
    g_object_unref (stream);
    if (byzanz_encoder_get_error (session->encoder))
      byzanz_session_set_error (session, byzanz_encoder_get_error (session->encoder));
  }
  byzanz_serialize_header (byzanz_queue_get_output_stream (session->queue),
      session->area.width, session->area.height, session->cancellable, &session->error);

  if (G_OBJECT_CLASS (byzanz_session_parent_class)->constructed)
    G_OBJECT_CLASS (byzanz_session_parent_class)->constructed (object);
}

static void
byzanz_session_class_init (ByzanzSessionClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = byzanz_session_get_property;
  object_class->set_property = byzanz_session_set_property;
  object_class->dispose = byzanz_session_dispose;
  object_class->finalize = byzanz_session_finalize;
  object_class->constructed = byzanz_session_constructed;

  g_object_class_install_property (object_class, PROP_ERROR,
      g_param_spec_pointer ("error", "error", "error that happened on the thread",
	  G_PARAM_READABLE));
  g_object_class_install_property (object_class, PROP_RECORDING,
      g_param_spec_boolean ("recording", "recording", "TRUE while the recorder is running",
	  FALSE, G_PARAM_READABLE));
  g_object_class_install_property (object_class, PROP_ENCODING,
      g_param_spec_boolean ("encoding", "encoding", "TRUE while the encoder is running",
	  TRUE, G_PARAM_READABLE));
  g_object_class_install_property (object_class, PROP_WINDOW,
      g_param_spec_object ("window", "window", "window to record from",
	  GDK_TYPE_WINDOW, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property (object_class, PROP_AREA,
      g_param_spec_boxed ("area", "area", "recorded area",
	  GDK_TYPE_RECTANGLE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property (object_class, PROP_FILE,
      g_param_spec_object ("file", "file", "file to record to",
	  G_TYPE_FILE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property (object_class, PROP_AUDIO,
      g_param_spec_boolean ("record-audio", "record audio", "TRUE to record audio",
	  FALSE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property (object_class, PROP_ENCODER_TYPE,
      g_param_spec_gtype ("encoder-type", "encoder type", "type for the encoder to use",
	  BYZANZ_TYPE_ENCODER, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
}

static void
byzanz_session_init (ByzanzSession *session)
{
  session->cancellable = g_cancellable_new ();
  session->queue = byzanz_queue_new ();
}

/**
 * byzanz_session_new:
 * @file: file to record to. Any existing file will be overwritten.
 * @encoder_type: the type of encoder to use
 * @window: window to record
 * @area: area of window that should be recorded
 * @record_cursor: if the cursor image should be recorded
 * @record_audio: if audio should be recorded
 *
 * Creates a new #ByzanzSession and initializes all basic variables. 
 * gtk_init() and g_thread_init() must have been called before.
 *
 * Returns: a new #ByzanzSession or NULL if an error occured. Most likely
 *          the XDamage extension is not available on the current X server
 *          then. Another reason would be a thread creation failure.
 **/
ByzanzSession *
byzanz_session_new (GFile *file, GType encoder_type, 
    GdkWindow *window, const cairo_rectangle_int_t *area, gboolean record_cursor,
    gboolean record_audio)
{
  g_return_val_if_fail (G_IS_FILE (file), NULL);
  g_return_val_if_fail (g_type_is_a (encoder_type, BYZANZ_TYPE_ENCODER), NULL);
  g_return_val_if_fail (GDK_IS_WINDOW (window), NULL);
  g_return_val_if_fail (area != NULL, NULL);
  g_return_val_if_fail (area->x >= 0, NULL);
  g_return_val_if_fail (area->y >= 0, NULL);
  g_return_val_if_fail (area->width > 0, NULL);
  g_return_val_if_fail (area->height > 0, NULL);
  
  /* FIXME: handle mouse cursor */

  return g_object_new (BYZANZ_TYPE_SESSION, "file", file, "encoder-type", encoder_type,
      "window", window, "area", area, "record-audio", record_audio, NULL);
}

void
byzanz_session_start (ByzanzSession *session)
{
  g_return_if_fail (BYZANZ_IS_SESSION (session));

  byzanz_recorder_set_recording (session->recorder, TRUE);
}

void
byzanz_session_stop (ByzanzSession *session)
{
  GOutputStream *stream;
  GError *error = NULL;
  GTimeVal tv;

  g_return_if_fail (BYZANZ_IS_SESSION (session));

  stream = byzanz_queue_get_output_stream (session->queue);
  g_get_current_time (&tv);
  if (!byzanz_serialize (stream, byzanz_session_elapsed (session, &tv), 
          NULL, NULL, session->cancellable, &error) || 
      !g_output_stream_close (stream, session->cancellable, &error)) {
    byzanz_session_set_error (session, error);
    g_error_free (error);
  }

  byzanz_recorder_set_recording (session->recorder, FALSE);
}

void
byzanz_session_abort (ByzanzSession *session)
{
  g_return_if_fail (BYZANZ_IS_SESSION (session));

  g_cancellable_cancel (session->cancellable);
}

gboolean
byzanz_session_is_recording (ByzanzSession *session)
{
  g_return_val_if_fail (BYZANZ_IS_SESSION (session), FALSE);

  return session->error == NULL &&
    byzanz_recorder_get_recording (session->recorder);
}

gboolean
byzanz_session_is_encoding (ByzanzSession *session)
{
  g_return_val_if_fail (BYZANZ_IS_SESSION (session), FALSE);

  return session->error == NULL && byzanz_encoder_is_running (session->encoder);
}

const GError *
byzanz_session_get_error (ByzanzSession *session)
{
  g_return_val_if_fail (BYZANZ_IS_SESSION (session), NULL);
  
  return session->error;
}

