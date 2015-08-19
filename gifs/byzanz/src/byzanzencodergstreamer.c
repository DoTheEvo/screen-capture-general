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
#include "config.h"
#endif

#include "byzanzencodergstreamer.h"

#include <glib/gi18n-lib.h>
#include <gst/video/video.h>

#include "byzanzserialize.h"

G_DEFINE_TYPE (ByzanzEncoderGStreamer, byzanz_encoder_gstreamer, BYZANZ_TYPE_ENCODER)

static void
byzanz_encoder_gstreamer_need_data (GstAppSrc *src, guint length, gpointer data)
{
  ByzanzEncoder *encoder = data;
  ByzanzEncoderGStreamer *gst = data;
  GstBuffer *buffer;
  cairo_t *cr;
  cairo_surface_t *surface;
  cairo_region_t *region;
  GError *error = NULL;
  guint64 msecs;
  int i, num_rects;

  if (!byzanz_deserialize (encoder->input_stream, &msecs, &surface, &region, encoder->cancellable, &error)) {
    gst_element_message_full (GST_ELEMENT (src), GST_MESSAGE_ERROR,
        error->domain, error->code, g_strdup (error->message), NULL, __FILE__, GST_FUNCTION, __LINE__);
    g_error_free (error);
    return;
  }

  if (surface == NULL) {
    gst_app_src_end_of_stream (gst->src);
    if (gst->audiosrc)
      gst_element_send_event (gst->audiosrc, gst_event_new_eos ());
    return;
  }

  if (cairo_surface_get_reference_count (gst->surface) > 1) {
    cairo_surface_t *copy = cairo_image_surface_create (CAIRO_FORMAT_RGB24,
        cairo_image_surface_get_width (gst->surface), cairo_image_surface_get_height (gst->surface));
    
    cr = cairo_create (copy);
    cairo_set_source_surface (cr, gst->surface, 0, 0);
    cairo_paint (cr);
    cairo_destroy (cr);

    cairo_surface_destroy (gst->surface);
    gst->surface = copy;
  }
  cr = cairo_create (gst->surface);
  cairo_set_source_surface (cr, surface, 0, 0);

  num_rects = cairo_region_num_rectangles (region);
  for (i = 0; i < num_rects; i++) {
    cairo_rectangle_int_t rect;
    cairo_region_get_rectangle (region, i, &rect);
    cairo_rectangle (cr, rect.x, rect.y,
                     rect.width, rect.height);
  }

  cairo_fill (cr);
  cairo_destroy (cr);

  /* create a buffer and send it */
  /* FIXME: stride just works? */
  cairo_surface_reference (gst->surface);
  buffer = gst_buffer_new_wrapped_full (GST_MEMORY_FLAG_READONLY,
                                        cairo_image_surface_get_data (gst->surface),
                                        cairo_image_surface_get_stride (gst->surface) * cairo_image_surface_get_height (gst->surface),
                                        0,
                                        cairo_image_surface_get_stride (gst->surface) * cairo_image_surface_get_height (gst->surface),
                                        gst->surface,
                                        (GDestroyNotify) cairo_surface_destroy);
  GST_BUFFER_TIMESTAMP (buffer) = msecs * GST_MSECOND;
  gst_app_src_push_buffer (gst->src, buffer);
}

static GstAppSrcCallbacks callbacks = {
  byzanz_encoder_gstreamer_need_data, NULL, NULL
};

static gboolean
byzanz_encoder_gstreamer_run (ByzanzEncoder * encoder,
                              GInputStream *  input,
                              GOutputStream * output,
                              gboolean        record_audio,
                              GCancellable *  cancellable,
                              GError **	      error)
{
  ByzanzEncoderGStreamer *gstreamer = BYZANZ_ENCODER_GSTREAMER (encoder);
  ByzanzEncoderGStreamerClass *klass = BYZANZ_ENCODER_GSTREAMER_GET_CLASS (encoder);
  GstElement *sink;
  guint width, height;
  GstMessage *message;
  GstBus *bus;

  if (!byzanz_deserialize_header (input, &width, &height, cancellable, error))
    return FALSE;

  gstreamer->surface = cairo_image_surface_create (CAIRO_FORMAT_RGB24, width, height);

  g_assert (klass->pipeline_string);
  if (record_audio) {
    if (klass->audio_pipeline_string == NULL) {
      g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_FAILED,
          _("This format does not support recording audio."));
      return FALSE;
    }
    gstreamer->pipeline = gst_parse_launch (klass->audio_pipeline_string, error);
    gstreamer->audiosrc = gst_bin_get_by_name (GST_BIN (gstreamer->pipeline), "audiosrc");
    g_assert (gstreamer->audiosrc);
  } else {
    gstreamer->pipeline = gst_parse_launch (klass->pipeline_string, error);
  }
  if (gstreamer->pipeline == NULL)
    return FALSE;

  g_assert (GST_IS_PIPELINE (gstreamer->pipeline));
  gstreamer->src = GST_APP_SRC (gst_bin_get_by_name (GST_BIN (gstreamer->pipeline), "src"));
  g_assert (GST_IS_APP_SRC (gstreamer->src));
  sink = gst_bin_get_by_name (GST_BIN (gstreamer->pipeline), "sink");
  g_assert (sink);
  g_object_set (sink, "stream", output, NULL);
  g_object_unref (sink);

  gstreamer->caps = gst_caps_new_simple ("video/x-raw",
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
                                         "format", G_TYPE_STRING, "BGRx",
#elif G_BYTE_ORDER == G_BIG_ENDIAN
                                         "format", G_TYPE_STRING, "xRGB",
#else
#error "Please add the Cairo caps format here"
#endif
                                         "width", G_TYPE_INT, width,
                                         "height", G_TYPE_INT, height,
                                         "framerate", GST_TYPE_FRACTION, 0, 1, NULL);
  g_assert (gst_caps_is_fixed (gstreamer->caps));

  gst_app_src_set_caps (gstreamer->src, gstreamer->caps);
  gst_app_src_set_callbacks (gstreamer->src, &callbacks, gstreamer, NULL);
  gst_app_src_set_stream_type (gstreamer->src, GST_APP_STREAM_TYPE_STREAM);
  gst_app_src_set_max_bytes (gstreamer->src, 0);
  g_object_set (gstreamer->src,
                "format", GST_FORMAT_TIME,
                NULL);

  if (!gst_element_set_state (gstreamer->pipeline, GST_STATE_PLAYING)) {
    g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_FAILED, _("Failed to start GStreamer pipeline"));
    return FALSE;
  }

  bus = gst_pipeline_get_bus (GST_PIPELINE (gstreamer->pipeline));
  message = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);
  g_object_unref (bus);

  gst_element_set_state (gstreamer->pipeline, GST_STATE_NULL);

  if (GST_MESSAGE_TYPE (message) == GST_MESSAGE_ERROR) {
    gst_message_parse_error (message, error, NULL);
    gst_message_unref (message);
    return FALSE;
  }
  gst_message_unref (message);

  return TRUE;
}

static void
byzanz_encoder_gstreamer_finalize (GObject *object)
{
  ByzanzEncoderGStreamer *gstreamer = BYZANZ_ENCODER_GSTREAMER (object);

  if (gstreamer->pipeline) {
    gst_element_set_state (gstreamer->pipeline, GST_STATE_NULL);
    g_object_unref (gstreamer->pipeline);
  }
  if (gstreamer->src)
    g_object_unref (gstreamer->src);
  if (gstreamer->caps)
    gst_caps_unref (gstreamer->caps);

  if (gstreamer->surface)
    cairo_surface_destroy (gstreamer->surface);

  G_OBJECT_CLASS (byzanz_encoder_gstreamer_parent_class)->finalize (object);
}

static void
byzanz_encoder_gstreamer_class_init (ByzanzEncoderGStreamerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ByzanzEncoderClass *encoder_class = BYZANZ_ENCODER_CLASS (klass);

  gst_init (NULL, NULL);

  object_class->finalize = byzanz_encoder_gstreamer_finalize;

  encoder_class->run = byzanz_encoder_gstreamer_run;
}

static void
byzanz_encoder_gstreamer_init (ByzanzEncoderGStreamer *encoder_gstreamer)
{
}

