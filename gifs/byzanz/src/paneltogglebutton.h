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


#ifndef __PANEL_TOGGLE_BUTTON_H__
#define __PANEL_TOGGLE_BUTTON_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS


#define PANEL_TYPE_TOGGLE_BUTTON            (panel_toggle_button_get_type ())
#define PANEL_TOGGLE_BUTTON(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PANEL_TYPE_TOGGLE_BUTTON, PanelToggleButton))
#define PANEL_TOGGLE_BUTTON_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PANEL_TYPE_TOGGLE_BUTTON, PanelToggleButtonClass))
#define PANEL_IS_TOGGLE_BUTTON(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PANEL_TYPE_TOGGLE_BUTTON))
#define PANEL_IS_TOGGLE_BUTTON_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PANEL_TYPE_TOGGLE_BUTTON))
#define PANEL_TOGGLE_BUTTON_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PANEL_TYPE_TOGGLE_BUTTON, PanelToggleButtonClass))

typedef struct _PanelToggleButton	    PanelToggleButton;
typedef struct _PanelToggleButtonClass	    PanelToggleButtonClass;

struct _PanelToggleButton
{
  GtkToggleButton	toggle_button;
};

struct _PanelToggleButtonClass
{
  GtkToggleButtonClass	toggle_button_class;
};

GType		panel_toggle_button_get_type	(void) G_GNUC_CONST;

GtkWidget *	panel_toggle_button_new		(void);


G_END_DECLS

#endif /* __PANEL_TOGGLE_BUTTON_H__ */
