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

#include "byzanzlayerwindow.h"

#include <gdk/gdkx.h>

G_DEFINE_TYPE (ByzanzLayerWindow, byzanz_layer_window, BYZANZ_TYPE_LAYER)

static gboolean
byzanz_layer_window_event (ByzanzLayer * layer,
                           GdkXEvent *   gdkxevent)
{
  XDamageNotifyEvent *event = (XDamageNotifyEvent *) gdkxevent;
  ByzanzLayerWindow *wlayer = BYZANZ_LAYER_WINDOW (layer);

  if (event->type == layer->recorder->damage_event_base + XDamageNotify && 
      event->damage == wlayer->damage) {
    cairo_rectangle_int_t rect;

    rect.x = event->area.x;
    rect.y = event->area.y;
    rect.width = event->area.width;
    rect.height = event->area.height;
    if (gdk_rectangle_intersect ((GdkRectangle*) &rect,
                                 (GdkRectangle*) &layer->recorder->area,
                                 (GdkRectangle*) &rect)) {
      cairo_region_union_rectangle (wlayer->invalid, &rect);
      byzanz_layer_invalidate (layer);
    }
    return TRUE;
  }

  return FALSE;
}

static XserverRegion
byzanz_server_region_from_gdk (Display *dpy, cairo_region_t *source)
{
  XserverRegion dest;
  XRectangle *dest_rects;
  cairo_rectangle_int_t source_rect;
  int n_rectangles, i;

  n_rectangles = cairo_region_num_rectangles (source);
  g_assert (n_rectangles);
  dest_rects = g_new (XRectangle, n_rectangles);
  for (i = 0; i < n_rectangles; i++) {
    cairo_region_get_rectangle (source, i, &source_rect);
    dest_rects[i].x = source_rect.x;
    dest_rects[i].y = source_rect.y;
    dest_rects[i].width = source_rect.width;
    dest_rects[i].height = source_rect.height;
  }
  dest = XFixesCreateRegion (dpy, dest_rects, n_rectangles);
  g_free (dest_rects);

  return dest;
}

static cairo_region_t *
byzanz_layer_window_snapshot (ByzanzLayer *layer)
{
  Display *dpy = GDK_DISPLAY_XDISPLAY (gdk_window_get_display (layer->recorder->window));
  ByzanzLayerWindow *wlayer = BYZANZ_LAYER_WINDOW (layer);
  XserverRegion reg;
  cairo_region_t *region;

  if (cairo_region_is_empty (wlayer->invalid))
    return NULL;

  reg = byzanz_server_region_from_gdk (dpy, wlayer->invalid);
  XDamageSubtract (dpy, wlayer->damage, reg, reg);
  XFixesDestroyRegion (dpy, reg);

  region = wlayer->invalid;
  wlayer->invalid = cairo_region_create ();
  return region;
}

static void
byzanz_layer_window_render (ByzanzLayer *layer,
                            cairo_t *    cr)
{
  gdk_cairo_set_source_window (cr, layer->recorder->window, 0, 0);
  cairo_paint (cr);
}

static void
byzanz_layer_window_finalize (GObject *object)
{
  Display *dpy = GDK_DISPLAY_XDISPLAY (gdk_window_get_display (BYZANZ_LAYER (object)->recorder->window));
  ByzanzLayerWindow *wlayer = BYZANZ_LAYER_WINDOW (object);

  XDamageDestroy (dpy, wlayer->damage);
  cairo_region_destroy (wlayer->invalid);

  G_OBJECT_CLASS (byzanz_layer_window_parent_class)->finalize (object);
}

static void
byzanz_layer_window_constructed (GObject *object)
{
  ByzanzLayer *layer = BYZANZ_LAYER (object);
  GdkWindow *window = layer->recorder->window;
  Display *dpy = GDK_DISPLAY_XDISPLAY (gdk_window_get_display (window));
  ByzanzLayerWindow *wlayer = BYZANZ_LAYER_WINDOW (object);

  wlayer->damage = XDamageCreate (dpy, gdk_x11_window_get_xid (window), XDamageReportDeltaRectangles);
  cairo_region_union_rectangle (wlayer->invalid, &layer->recorder->area);

  G_OBJECT_CLASS (byzanz_layer_window_parent_class)->constructed (object);
}

static void
byzanz_layer_window_class_init (ByzanzLayerWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ByzanzLayerClass *layer_class = BYZANZ_LAYER_CLASS (klass);

  object_class->finalize = byzanz_layer_window_finalize;
  object_class->constructed = byzanz_layer_window_constructed;

  layer_class->event = byzanz_layer_window_event;
  layer_class->snapshot = byzanz_layer_window_snapshot;
  layer_class->render = byzanz_layer_window_render;
}

static void
byzanz_layer_window_init (ByzanzLayerWindow *wlayer)
{
  wlayer->invalid = cairo_region_create ();
}

