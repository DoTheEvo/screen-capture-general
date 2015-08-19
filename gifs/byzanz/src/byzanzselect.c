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

#include "byzanzselect.h"

#include <glib/gi18n.h>

#include "screenshot-utils.h"

static void
rectangle_sanitize (cairo_rectangle_int_t *dest, const cairo_rectangle_int_t *src)
{
  *dest = *src;
  if (dest->width < 0) {
    dest->x += dest->width;
    dest->width = -dest->width;
  }
  if (dest->height < 0) {
    dest->y += dest->height;
    dest->height = -dest->height;
  }
}

typedef struct _ByzanzSelectData ByzanzSelectData;
struct _ByzanzSelectData {
  ByzanzSelectFunc      func;           /* func passed to byzanz_select_method_select() */
  gpointer              func_data;      /* data passed to byzanz_select_method_select() */

  /* results */
  GdkWindow *           result;         /* window that was selected */
  cairo_rectangle_int_t area;           /* the area to select */

  /* method data */
  GtkWidget *           window;         /* window we created to do selecting or NULL */
  cairo_surface_t *     root;           /* only used without XComposite, NULL otherwise: the root window */
};

static void
byzanz_select_data_free (gpointer datap)
{
  ByzanzSelectData *data = datap;

  g_assert (data->window == NULL);

  if (data->root)
    cairo_surface_destroy (data->root);
  if (data->result)
    g_object_unref (data->result);

  g_slice_free (ByzanzSelectData, data);
}

static gboolean
byzanz_select_really_done (gpointer datap)
{
  ByzanzSelectData *data = datap;

  data->func (data->result, &data->area, data->func_data);
  byzanz_select_data_free (data);

  return FALSE;
}

static void
byzanz_select_done (ByzanzSelectData *data, GdkWindow *window)
{
  if (data->window) {
    gtk_widget_destroy (data->window);
    data->window = NULL;
  }

  if (window) {
    /* stupid hack to get around a session recording the selection window */
    gdk_display_sync (gdk_window_get_display (window));
    data->result = g_object_ref (window);
    gdk_threads_add_timeout (1000, byzanz_select_really_done, data);
  } else {
    byzanz_select_really_done (data);
  }
}

/*** SELECT AREA ***/

/* define for SLOW selection mechanism */
#undef TARGET_LINE

static gboolean
expose_cb (GtkWidget *widget, GdkEventExpose *event, gpointer datap)
{
  cairo_t *cr;
  ByzanzSelectData *data = datap;
#ifdef TARGET_LINE
  static double dashes[] = { 1.0, 2.0 };
#endif

  cr = gdk_cairo_create (gtk_widget_get_window (widget));
  cairo_rectangle (cr, event->area.x, event->area.y, event->area.width, event->area.height);
  cairo_clip (cr);

  /* clear (or draw) background */
  cairo_save (cr);
  if (data->root) {
    cairo_set_source_surface (cr, data->root, 0, 0);
  } else {
    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 0.0);
  }
  cairo_paint (cr);
  cairo_restore (cr);

  /* FIXME: make colors use theme */
  cairo_set_line_width (cr, 1.0);
#ifdef TARGET_LINE
  cairo_set_source_rgba (cr, 1.0, 0.0, 0.0, 1.0);
  cairo_set_dash (cr, dashes, G_N_ELEMENTS (dashes), 0.0);
  cairo_move_to (cr, data->area.x + data->area.width - 0.5, 0.0);
  cairo_line_to (cr, data->area.x + data->area.width - 0.5, event->area.y + event->area.height); /* end of screen really */
  cairo_move_to (cr, 0.0, data->area.y + data->area.height - 0.5);
  cairo_line_to (cr, event->area.x + event->area.width, data->area.y + data->area.height - 0.5); /* end of screen really */
  cairo_stroke (cr);
#endif
  if (data->area.x >= 0 && data->area.width != 0 && data->area.height != 0) {
    cairo_rectangle_int_t rect = data->area;
    rectangle_sanitize (&rect, &data->area);
    cairo_set_source_rgba (cr, 0.0, 0.0, 0.5, 0.2);
    cairo_set_dash (cr, NULL, 0, 0.0);
    gdk_cairo_rectangle (cr, (GdkRectangle *) &rect);
    cairo_fill (cr);
    cairo_set_source_rgba (cr, 0.0, 0.0, 0.5, 0.5);
    cairo_rectangle (cr, rect.x + 0.5, rect.y + 0.5, rect.width - 1, rect.height - 1);
    cairo_stroke (cr);
  }
  if (cairo_status (cr) != CAIRO_STATUS_SUCCESS)
    g_printerr ("cairo error: %s\n", cairo_status_to_string (cairo_status (cr)));
  cairo_destroy (cr);
  return FALSE;
}

static gboolean
button_pressed_cb (GtkWidget *widget, GdkEventButton *event, gpointer datap)
{
  ByzanzSelectData *data = datap;

  if (event->button != 1) {
    byzanz_select_done (data, NULL);
    return TRUE;
  }
  data->area.x = event->x;
  data->area.y = event->y;
  data->area.width = 1;
  data->area.height = 1;

  gtk_widget_queue_draw (widget);

  return TRUE;
}

static gboolean
button_released_cb (GtkWidget *widget, GdkEventButton *event, gpointer datap)
{
  ByzanzSelectData *data = datap;
  
  if (event->button == 1 && data->area.x >= 0) {
    data->area.width = event->x - data->area.x;
    data->area.height = event->y - data->area.y;
    rectangle_sanitize (&data->area, &data->area);
    byzanz_select_done (data, gdk_get_default_root_window ());
  }
  
  return TRUE;
}

static gboolean
motion_notify_cb (GtkWidget *widget, GdkEventMotion *event, gpointer datap)
{
  ByzanzSelectData *data = datap;
  
#ifdef TARGET_LINE
  gtk_widget_queue_draw (widget);
#else
  if (data->area.x >= 0) {
    cairo_rectangle_int_t rect;
    rectangle_sanitize (&rect, &data->area);
    gtk_widget_queue_draw_area (widget, rect.x, rect.y, rect.width, rect.height);
  }
#endif
  data->area.width = event->x - data->area.x;
  data->area.height = event->y - data->area.y;
  if (data->area.x >= 0) {
    cairo_rectangle_int_t rect;
    rectangle_sanitize (&rect, &data->area);
    gtk_widget_queue_draw_area (widget, rect.x, rect.y, rect.width, rect.height);
  }

  return TRUE;
}

static void
realize_cb (GtkWidget *widget, gpointer datap)
{
  GdkWindow *window = gtk_widget_get_window (widget);
  GdkCursor *cursor;

  gdk_window_set_events (window, gdk_window_get_events (window) |
      GDK_BUTTON_PRESS_MASK |
      GDK_BUTTON_RELEASE_MASK |
      GDK_POINTER_MOTION_MASK);
  cursor = gdk_cursor_new (GDK_CROSSHAIR);
  gdk_window_set_cursor (window, cursor);
  g_object_unref (cursor);
  gdk_window_set_background_pattern (window, NULL);
}

static void
delete_cb (GtkWidget *widget, ByzanzSelectData *data)
{
  byzanz_select_done (data, NULL);
}

static void
active_cb (GtkWindow *window, GParamSpec *pspec, ByzanzSelectData *data)
{
  if (!gtk_window_is_active (window))
    byzanz_select_done (data, NULL);
}

static void
byzanz_select_area (ByzanzSelectData *data)
{
  GdkVisual *visual;
  
  data->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  data->area.x = -1;
  data->area.y = -1;

  visual = gdk_screen_get_rgba_visual (gdk_screen_get_default ());
  if (visual && gdk_screen_is_composited (gdk_screen_get_default ())) {
    gtk_widget_set_visual (data->window, visual);
  } else {
    GdkWindow *root = gdk_get_default_root_window ();
    cairo_t *cr;
    cairo_surface_t *root_surface;
    gint width, height;
    width = gdk_window_get_width (root);
    height = gdk_window_get_height (root);

    cr = gdk_cairo_create (root);
    root_surface = cairo_surface_reference (cairo_get_target (cr));
    cairo_destroy (cr);

    data->root = cairo_surface_create_similar (root_surface, CAIRO_CONTENT_COLOR, width, height);
    cr = cairo_create (data->root);
    cairo_set_source_surface (cr, root_surface, 0, 0);
    cairo_paint (cr);
    cairo_destroy (cr);
    cairo_surface_destroy (root_surface);
  }
  gtk_widget_set_app_paintable (data->window, TRUE);
  gtk_window_fullscreen (GTK_WINDOW (data->window));
  g_signal_connect (data->window, "expose-event", G_CALLBACK (expose_cb), data);
  g_signal_connect (data->window, "button-press-event", G_CALLBACK (button_pressed_cb), data);
  g_signal_connect (data->window, "button-release-event", G_CALLBACK (button_released_cb), data);
  g_signal_connect (data->window, "motion-notify-event", G_CALLBACK (motion_notify_cb), data);
  g_signal_connect (data->window, "delete-event", G_CALLBACK (delete_cb), data);
  g_signal_connect (data->window, "notify::is-active", G_CALLBACK (active_cb), data);
  g_signal_connect_after (data->window, "realize", G_CALLBACK (realize_cb), data);
  gtk_widget_show_all (data->window);
}

/*** WHOLE SCREEN ***/

static void
byzanz_select_screen (ByzanzSelectData *data)
{
  GdkWindow *root;
  
  root = gdk_get_default_root_window ();
  data->area.width = gdk_window_get_width (root);
  data->area.height = gdk_window_get_height (root);
  byzanz_select_done (data, root);
}

/*** APPLICATION WINDOW ***/

static gboolean
select_window_button_pressed_cb (GtkWidget *widget, GdkEventButton *event, gpointer datap)
{
  ByzanzSelectData *data = datap;
  GdkWindow *window;

  gdk_device_ungrab (gdk_event_get_device ((GdkEvent*)event), event->time);
  if (event->button == 1) {
    Window w;

    w = screenshot_find_current_window (TRUE);
    if (w != None)
      window = gdk_x11_window_foreign_new_for_display (gdk_display_get_default (), w);
    else
      window = gdk_get_default_root_window ();

    gdk_window_get_root_origin (window, &data->area.x, &data->area.y);
    data->area.width = gdk_window_get_width (window);
    data->area.height = gdk_window_get_height (window);
    g_object_unref (window);

    window = gdk_get_default_root_window ();
  } else {
    window = NULL;
  }
  byzanz_select_done (data, window);
  return TRUE;
}

static void
byzanz_select_window (ByzanzSelectData *data)
{
  GdkCursor *cursor;
  GdkWindow *window;
  GdkDevice *device;
  GdkDeviceManager *device_manager;
  GdkDisplay *display;

  cursor = gdk_cursor_new (GDK_CROSSHAIR);
  data->window = gtk_invisible_new ();
  g_signal_connect (data->window, "button-press-event", 
      G_CALLBACK (select_window_button_pressed_cb), data);
  gtk_widget_show (data->window);
  window = gtk_widget_get_window (data->window);
  display = gdk_window_get_display (window);
  device_manager = gdk_display_get_device_manager (display);
  device = gdk_device_manager_get_client_pointer (device_manager);
  gdk_device_grab (device, window, GDK_OWNERSHIP_NONE, FALSE, GDK_BUTTON_PRESS_MASK, cursor, GDK_CURRENT_TIME);
  g_object_unref (cursor);
}
  
/*** API ***/

static const struct {
  const char * mnemonic;
  const char * description;
  const char * icon_name;
  const char * method_name;
  void (* select) (ByzanzSelectData *data);
} methods [] = {
  { N_("Record _Desktop"), N_("Record the entire desktop"), 
    "byzanz-record-desktop", "screen", byzanz_select_screen },
  { N_("Record _Area"), N_("Record a selected area of the desktop"), 
    "byzanz-record-area", "area", byzanz_select_area },
  { N_("Record _Window"), N_("Record a selected window"), 
    "byzanz-record-window", "window", byzanz_select_window }
};
#define BYZANZ_METHOD_COUNT G_N_ELEMENTS(methods)

guint
byzanz_select_get_method_count (void)
{
  return BYZANZ_METHOD_COUNT;
}

const char *
byzanz_select_method_get_icon_name (guint method)
{
  g_return_val_if_fail (method < BYZANZ_METHOD_COUNT, NULL);

  return methods[method].icon_name;
}

const char *
byzanz_select_method_get_name (guint method)
{
  g_return_val_if_fail (method < BYZANZ_METHOD_COUNT, NULL);

  return methods[method].method_name;
}

int
byzanz_select_method_lookup (const char *name)
{
  guint i;

  g_return_val_if_fail (name != NULL, -1);

  for (i = 0; i < BYZANZ_METHOD_COUNT; i++) {
    if (g_str_equal (name, methods[i].method_name))
      return i;
  }
  return -1;
}

const char *
byzanz_select_method_describe (guint method)
{
  g_return_val_if_fail (method < BYZANZ_METHOD_COUNT, NULL);

  return _(methods[method].description);
}

const char *
byzanz_select_method_get_mnemonic (guint method)
{
  g_return_val_if_fail (method < BYZANZ_METHOD_COUNT, NULL);

  return _(methods[method].mnemonic);
}

void
byzanz_select_method_select (guint               method,
                             ByzanzSelectFunc    func,
                             gpointer            func_data)
{
  ByzanzSelectData *data;

  g_return_if_fail (method < BYZANZ_METHOD_COUNT);
  g_return_if_fail (func != NULL);

  g_assert (methods[method].select != NULL);

  data = g_slice_new0 (ByzanzSelectData);
  data->func = func;
  data->func_data = func_data;

  methods[method].select (data);
}

