/*
 * Copyright (C) 2005 by Benjamin Otte <otte@gnome.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 $Id$
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "paneltogglebutton.h"

static GtkToggleButtonClass *parent_class = NULL;


static void 
panel_toggle_button_size_request (GtkWidget *widget, 
    GtkRequisition *requisition)
{
  GtkWidget *child;
  
  child = gtk_bin_get_child (GTK_BIN (widget));

  if (child) {
    gtk_widget_get_preferred_size (child, requisition, NULL);
  } else {
    requisition->width = requisition->height = 0;
  }
}

static void
panel_toggle_button_get_preferred_width (GtkWidget *widget,
                                         gint      *minimal_width,
                                         gint      *natural_width)
{
  GtkRequisition requisition;

  panel_toggle_button_size_request (widget, &requisition);

  *minimal_width = *natural_width = requisition.width;
}

static void
panel_toggle_button_get_preferred_height (GtkWidget *widget,
                                          gint      *minimal_height,
                                          gint      *natural_height)
{
  GtkRequisition requisition;

  panel_toggle_button_size_request (widget, &requisition);

  *minimal_height = *natural_height = requisition.height;
}

static void
panel_toggle_button_size_allocate (GtkWidget *widget,
    GtkAllocation *allocation)
{
  GtkWidget *child;
  
  child = gtk_bin_get_child (GTK_BIN (widget));

  gtk_widget_set_allocation (widget, allocation);
  
  if (gtk_widget_get_realized (widget))
    gdk_window_move_resize (gtk_button_get_event_window (GTK_BUTTON(widget)), 
	allocation->x, allocation->y,
	allocation->width, allocation->height); 
  
  if (child)
    gtk_widget_size_allocate (child, allocation);
}

static gboolean
panel_toggle_button_draw (GtkWidget *widget, cairo_t *cr)
{
  GtkWidget *child = gtk_bin_get_child (GTK_BIN (widget));
  GtkStateType state_type;
  GtkShadowType shadow_type;
  GtkAllocation allocation;
  GtkStyleContext *context = gtk_widget_get_style_context (widget);
  GtkStateFlags flags = 0;

  state_type = gtk_widget_get_state (widget);
    
  /* FIXME: someone make this layout work nicely for all themes
   * Currently I'm trying to imitate the volume applet's widget */
  if (gtk_toggle_button_get_inconsistent (GTK_TOGGLE_BUTTON (widget))) {
    if (state_type == GTK_STATE_ACTIVE)
      state_type = GTK_STATE_NORMAL;
    shadow_type = GTK_SHADOW_ETCHED_IN;
  } else {
    shadow_type = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)) ? GTK_SHADOW_IN : GTK_SHADOW_OUT;
  }
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
    state_type = GTK_STATE_SELECTED;
  /* FIXME: better detail? */
  gtk_widget_get_allocation (widget, &allocation);
  gtk_style_context_add_class (context, "togglebutton");
  switch (state_type)
    {
    case GTK_STATE_PRELIGHT:
      flags |= GTK_STATE_FLAG_PRELIGHT;
      break;
    case GTK_STATE_SELECTED:
      flags |= GTK_STATE_FLAG_SELECTED;
      break;
    case GTK_STATE_INSENSITIVE:
      flags |= GTK_STATE_FLAG_INSENSITIVE;
      break;
    case GTK_STATE_ACTIVE:
      flags |= GTK_STATE_FLAG_ACTIVE;
      break;
    case GTK_STATE_FOCUSED:
      flags |= GTK_STATE_FLAG_FOCUSED;
      break;
    case GTK_STATE_NORMAL:
    case GTK_STATE_INCONSISTENT:
    default:
      break;
  }

  gtk_style_context_set_state (context, flags);
  gtk_render_background (context, cr, (gdouble) allocation.x, (gdouble) allocation.y,
                                 (gdouble) allocation.width, (gdouble) allocation.height);
  (void) shadow_type;

  if (child)
    gtk_container_propagate_draw (GTK_CONTAINER (widget), child, cr);
  
  return FALSE;
}

static gboolean
panel_toggle_button_button_press (GtkWidget *widget, GdkEventButton *event)
{
  if (event->button == 3 || event->button == 2)
    return FALSE;

  return GTK_WIDGET_CLASS (parent_class)->button_press_event (widget, event);
}

static gboolean
panel_toggle_button_button_release (GtkWidget *widget, GdkEventButton *event)
{
  if (event->button == 3 || event->button == 2)
    return FALSE;

  return GTK_WIDGET_CLASS (parent_class)->button_release_event (widget, event);
}

static void
panel_toggle_button_class_init (PanelToggleButtonClass *class)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (class);

  parent_class = g_type_class_peek_parent (class);

  widget_class->get_preferred_width = panel_toggle_button_get_preferred_width;
  widget_class->get_preferred_height = panel_toggle_button_get_preferred_height;
  widget_class->size_allocate = panel_toggle_button_size_allocate;
  widget_class->draw = panel_toggle_button_draw;
  widget_class->button_press_event = panel_toggle_button_button_press;
  widget_class->button_release_event = panel_toggle_button_button_release;
}

static void
panel_toggle_button_init (PanelToggleButton *toggle_button)
{
}

GType
panel_toggle_button_get_type (void)
{
  static GType toggle_button_type = 0;

  if (!toggle_button_type)
    {
      static const GTypeInfo toggle_button_info =
      {
	sizeof (PanelToggleButtonClass),
	NULL,		/* base_init */
	NULL,		/* base_finalize */
	(GClassInitFunc) panel_toggle_button_class_init,
	NULL,		/* class_finalize */
	NULL,		/* class_data */
	sizeof (PanelToggleButton),
	0,		/* n_preallocs */
	(GInstanceInitFunc) panel_toggle_button_init,
	NULL,		/* value_table */
      };

      toggle_button_type = g_type_register_static (GTK_TYPE_TOGGLE_BUTTON, "PanelToggleButton", 
					 &toggle_button_info, 0);
    }

  return toggle_button_type;
}

GtkWidget *
panel_toggle_button_new (void)
{
  return GTK_WIDGET (g_object_new (PANEL_TYPE_TOGGLE_BUTTON, NULL));
}

