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

#include "byzanzrecorder.h"

#include <gdk/gdkx.h>

#include <X11/extensions/Xdamage.h>
#include <X11/extensions/Xfixes.h>

#include "byzanzlayer.h"
#include "byzanzlayercursor.h"
#include "byzanzlayerwindow.h"

enum {
  PROP_0,
  PROP_WINDOW,
  PROP_AREA,
  PROP_RECORDING,
};

enum {
  IMAGE,
  LAST_SIGNAL
};

G_DEFINE_TYPE (ByzanzRecorder, byzanz_recorder, G_TYPE_OBJECT)
static guint signals[LAST_SIGNAL] = { 0, };

static cairo_region_t *
byzanz_recorder_get_invalid_region (ByzanzRecorder *recorder)
{
  cairo_region_t *invalid, *layer_invalid;
  GSequenceIter *iter;

  invalid = cairo_region_create ();
  for (iter = g_sequence_get_begin_iter (recorder->layers);
       !g_sequence_iter_is_end (iter);
       iter = g_sequence_iter_next (iter)) {
    ByzanzLayer *layer = g_sequence_get (iter);
    ByzanzLayerClass *klass = BYZANZ_LAYER_GET_CLASS (layer);

    layer_invalid = klass->snapshot (layer);
    if (layer_invalid) {
      cairo_region_union (invalid, layer_invalid);
      cairo_region_destroy (layer_invalid);
    }
  }

  return invalid;
}

static cairo_surface_t *
ensure_image_surface (cairo_surface_t *surface, const cairo_region_t *region)
{
  cairo_rectangle_int_t extents;
  cairo_surface_t *image;
  cairo_t *cr;
  int i, num_rects;

  if (cairo_surface_get_type (surface) == CAIRO_SURFACE_TYPE_IMAGE)
    return surface;

  cairo_region_get_extents (region, &extents);
  image = cairo_image_surface_create (CAIRO_FORMAT_RGB24, extents.width, extents.height);
  cairo_surface_set_device_offset (image, -extents.x, -extents.y);

  cr = cairo_create (image);
  cairo_set_source_surface (cr, surface, 0, 0);
  cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);

  num_rects = cairo_region_num_rectangles (region);
  for (i = 0; i < num_rects; i++) {
    cairo_rectangle_int_t rect;
    cairo_region_get_rectangle (region, i, &rect);
    cairo_rectangle (cr, rect.x, rect.y,
                     rect.width, rect.height);
  }

  cairo_fill (cr);
  cairo_destroy (cr);

  cairo_surface_destroy (surface);
  return image;
}

static cairo_surface_t *
byzanz_recorder_create_snapshot (ByzanzRecorder *recorder, const cairo_region_t *invalid)
{
  cairo_rectangle_int_t extents;
  cairo_surface_t *surface;
  cairo_t *cr;
  GSequenceIter *iter;
  int i, num_rects;
  
  cairo_region_get_extents (invalid, &extents);
  cr = gdk_cairo_create (recorder->window);
  surface = cairo_surface_create_similar (cairo_get_target (cr), CAIRO_CONTENT_COLOR,
      extents.width, extents.height);
  cairo_destroy (cr);
  cairo_surface_set_device_offset (surface, -extents.x, -extents.y);

  cr = cairo_create (surface);

  num_rects = cairo_region_num_rectangles (invalid);
  for (i = 0; i < num_rects; i++) {
    cairo_rectangle_int_t rect;
    cairo_region_get_rectangle (invalid, i, &rect);
    cairo_rectangle (cr, rect.x, rect.y,
                     rect.width, rect.height);
  }

  cairo_clip (cr);

  for (iter = g_sequence_get_begin_iter (recorder->layers);
       !g_sequence_iter_is_end (iter);
       iter = g_sequence_iter_next (iter)) {
    ByzanzLayer *layer = g_sequence_get (iter);
    ByzanzLayerClass *klass = BYZANZ_LAYER_GET_CLASS (layer);

    cairo_save (cr);
    klass->render (layer, cr);
    if (cairo_status (cr))
      g_critical ("error capturing image: %s", cairo_status_to_string (cairo_status (cr)));
    cairo_restore (cr);
  }

  cairo_destroy (cr);

  surface = ensure_image_surface (surface, invalid);

  /* adjust device offset here - the layers work in GdkScreen coordinates, the rest
   * of the code works in coordinates realtive to the passed in area. */
  cairo_surface_set_device_offset (surface,
      recorder->area.x - extents.x, recorder->area.y - extents.y);

  return surface;
}

static gboolean byzanz_recorder_snapshot (ByzanzRecorder *recorder);
static gboolean
byzanz_recorder_next_image (gpointer data)
{
  ByzanzRecorder *recorder = data;

  g_object_ref (recorder);
  recorder->next_image_source = 0;
  byzanz_recorder_snapshot (recorder);
  g_object_unref (recorder);
  return FALSE;
}

static gboolean
byzanz_recorder_snapshot (ByzanzRecorder *recorder)
{
  cairo_surface_t *surface;
  cairo_region_t *invalid;
  GTimeVal tv;

  if (!recorder->recording)
    return FALSE;

  if (recorder->next_image_source != 0)
    return FALSE;

  invalid = byzanz_recorder_get_invalid_region (recorder);
  if (cairo_region_is_empty (invalid)) {
    cairo_region_destroy (invalid);
    return FALSE;
  }

  surface = byzanz_recorder_create_snapshot (recorder, invalid);
  g_get_current_time (&tv);
  cairo_region_translate (invalid, -recorder->area.x, -recorder->area.y);

  g_signal_emit (recorder, signals[IMAGE], 0, surface, invalid, &tv);

  cairo_surface_destroy (surface);
  cairo_region_destroy (invalid);

  recorder->next_image_source = gdk_threads_add_timeout_full (G_PRIORITY_HIGH_IDLE,
      BYZANZ_RECORDER_FRAME_RATE_MS, byzanz_recorder_next_image, recorder, NULL);

  return TRUE;
}

static GdkFilterReturn
byzanz_recorder_filter_events (GdkXEvent *xevent, GdkEvent *event, gpointer data)
{
  ByzanzRecorder *recorder = data;
  GSequenceIter *iter;
  gboolean handled;

  if (event->any.window != recorder->window)
    return GDK_FILTER_CONTINUE;

  handled = FALSE;
  for (iter = g_sequence_get_begin_iter (recorder->layers);
       !g_sequence_iter_is_end (iter);
       iter = g_sequence_iter_next (iter)) {
    ByzanzLayer *layer = g_sequence_get (iter);
    ByzanzLayerClass *klass = BYZANZ_LAYER_GET_CLASS (layer);

    handled |= klass->event (layer, xevent);
  }

  return handled ? GDK_FILTER_REMOVE : GDK_FILTER_CONTINUE;
}

static gboolean
byzanz_recorder_prepare (ByzanzRecorder *recorder)
{
  GdkDisplay *display;
  Display *dpy;

  display = gdk_display_get_default ();
  dpy = gdk_x11_display_get_xdisplay (display);

  if (!XDamageQueryExtension (dpy, &recorder->damage_event_base, &recorder->damage_error_base) ||
      !XFixesQueryExtension (dpy, &recorder->fixes_event_base, &recorder->fixes_error_base))
    return FALSE;
  gdk_x11_register_standard_event_type (display, 
      recorder->damage_event_base + XDamageNotify, 1);
  gdk_x11_register_standard_event_type (display,
      recorder->fixes_event_base + XFixesCursorNotify, 1);

  return TRUE;
}

static void
byzanz_recorder_set_window (ByzanzRecorder *recorder, GdkWindow *window)
{
  g_assert (window != NULL);

  if (!byzanz_recorder_prepare (recorder)) {
    g_assert_not_reached ();
  }

  recorder->window = g_object_ref (window);
  gdk_window_add_filter (window, byzanz_recorder_filter_events, recorder);
}

static void
byzanz_recorder_set_property (GObject *object, guint param_id, const GValue *value, 
    GParamSpec * pspec)
{
  ByzanzRecorder *recorder = BYZANZ_RECORDER (object);

  switch (param_id) {
    case PROP_WINDOW:
      byzanz_recorder_set_window (recorder, g_value_get_object (value));
      break;
    case PROP_AREA:
      recorder->area = *(cairo_rectangle_int_t *) g_value_get_boxed (value);
      break;
    case PROP_RECORDING:
      byzanz_recorder_set_recording (recorder, g_value_get_boolean (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
  }
}

static void
byzanz_recorder_get_property (GObject *object, guint param_id, GValue *value, 
    GParamSpec * pspec)
{
  ByzanzRecorder *recorder = BYZANZ_RECORDER (object);

  switch (param_id) {
    case PROP_WINDOW:
      g_value_set_object (value, recorder->window);
      break;
    case PROP_AREA:
      g_value_set_boxed (value, &recorder->area);
      break;
    case PROP_RECORDING:
      g_value_set_boolean (value, byzanz_recorder_get_recording (recorder));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
  }
}

static void
byzanz_recorder_constructed (GObject *object)
{
  ByzanzRecorder *recorder = BYZANZ_RECORDER (object);

  g_sequence_append (recorder->layers,
      g_object_new (BYZANZ_TYPE_LAYER_WINDOW, "recorder", recorder, NULL));
  g_sequence_append (recorder->layers,
      g_object_new (BYZANZ_TYPE_LAYER_CURSOR, "recorder", recorder, NULL));

  if (G_OBJECT_CLASS (byzanz_recorder_parent_class)->constructed)
    G_OBJECT_CLASS (byzanz_recorder_parent_class)->constructed (object);
}

static void
byzanz_recorder_dispose (GObject *object)
{
  ByzanzRecorder *recorder = BYZANZ_RECORDER (object);

  g_sequence_remove_range (g_sequence_get_begin_iter (recorder->layers),
      g_sequence_get_end_iter (recorder->layers));

  G_OBJECT_CLASS (byzanz_recorder_parent_class)->dispose (object);
}

static void
byzanz_recorder_finalize (GObject *object)
{
  ByzanzRecorder *recorder = BYZANZ_RECORDER (object);

  if (recorder->next_image_source)
    g_source_remove (recorder->next_image_source);

  gdk_window_remove_filter (recorder->window, 
      byzanz_recorder_filter_events, recorder);
  g_object_unref (recorder->window);
  g_sequence_free (recorder->layers);

  G_OBJECT_CLASS (byzanz_recorder_parent_class)->finalize (object);
}

static void
byzanz_recorder_class_init (ByzanzRecorderClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = byzanz_recorder_get_property;
  object_class->set_property = byzanz_recorder_set_property;
  object_class->dispose = byzanz_recorder_dispose;
  object_class->finalize = byzanz_recorder_finalize;
  object_class->constructed = byzanz_recorder_constructed;

  g_object_class_install_property (object_class, PROP_WINDOW,
      g_param_spec_object ("window", "window", "window to record from",
	  GDK_TYPE_WINDOW, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property (object_class, PROP_AREA,
      g_param_spec_boxed ("area", "area", "recorded area",
	  GDK_TYPE_RECTANGLE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property (object_class, PROP_RECORDING,
      g_param_spec_boolean ("recording", "recording", "TRUE when actively recording",
	  FALSE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

  signals[IMAGE] = g_signal_new ("image", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (ByzanzRecorderClass, image), NULL, NULL, NULL,
      G_TYPE_NONE, 3, 
      G_TYPE_POINTER, G_TYPE_POINTER, G_TYPE_POINTER);
}

static void
byzanz_recorder_init (ByzanzRecorder *recorder)
{
  recorder->layers = g_sequence_new (g_object_unref);
}

ByzanzRecorder *
byzanz_recorder_new (GdkWindow *window, cairo_rectangle_int_t *area)
{
  g_return_val_if_fail (GDK_IS_WINDOW (window), NULL);
  g_return_val_if_fail (area != NULL, NULL);

  return g_object_new (BYZANZ_TYPE_RECORDER, "window", window, "area", area, NULL);
}

void
byzanz_recorder_set_recording (ByzanzRecorder *recorder, gboolean recording)
{
  g_return_if_fail (BYZANZ_IS_RECORDER (recorder));

  if (recorder->recording == recording)
    return;

  recorder->recording = recording;
  if (recording)
    byzanz_recorder_snapshot (recorder);
  g_object_notify (G_OBJECT (recorder), "recording");
}

gboolean
byzanz_recorder_get_recording (ByzanzRecorder *recorder)
{
  g_return_val_if_fail (BYZANZ_IS_RECORDER (recorder), FALSE);

  return recorder->recording;
}

void
byzanz_recorder_queue_snapshot (ByzanzRecorder *recorder)
{
  g_return_if_fail (BYZANZ_IS_RECORDER (recorder));

  if (recorder->next_image_source == 0) {
    recorder->next_image_source = gdk_threads_add_idle_full (G_PRIORITY_HIGH_IDLE,
        byzanz_recorder_next_image, recorder, NULL);
  }
}

