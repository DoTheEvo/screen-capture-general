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

#include "byzanzlayer.h"

#include <X11/extensions/Xdamage.h>
#include <X11/extensions/Xfixes.h>

#ifndef __HAVE_BYZANZ_LAYER_WINDOW_H__
#define __HAVE_BYZANZ_LAYER_WINDOW_H__

typedef struct _ByzanzLayerWindow ByzanzLayerWindow;
typedef struct _ByzanzLayerWindowClass ByzanzLayerWindowClass;

#define BYZANZ_TYPE_LAYER_WINDOW                    (byzanz_layer_window_get_type())
#define BYZANZ_IS_LAYER_WINDOW(obj)                 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BYZANZ_TYPE_LAYER_WINDOW))
#define BYZANZ_IS_LAYER_WINDOW_CLASS(klass)         (G_TYPE_CHECK_CLASS_TYPE ((klass), BYZANZ_TYPE_LAYER_WINDOW))
#define BYZANZ_LAYER_WINDOW(obj)                    (G_TYPE_CHECK_INSTANCE_CAST ((obj), BYZANZ_TYPE_LAYER_WINDOW, ByzanzLayerWindow))
#define BYZANZ_LAYER_WINDOW_CLASS(klass)            (G_TYPE_CHECK_CLASS_CAST ((klass), BYZANZ_TYPE_LAYER_WINDOW, ByzanzLayerWindowClass))
#define BYZANZ_LAYER_WINDOW_GET_CLASS(obj)          (G_TYPE_INSTANCE_GET_CLASS ((obj), BYZANZ_TYPE_LAYER_WINDOW, ByzanzLayerWindowClass))

struct _ByzanzLayerWindow {
  ByzanzLayer           layer;

  cairo_region_t *      invalid;                /* TRUE if we need to repaint */
  Damage		damage;		        /* the Damage object */
};

struct _ByzanzLayerWindowClass {
  ByzanzLayerClass	layer_class;
};

GType		        byzanz_layer_window_get_type	        (void) G_GNUC_CONST;


#endif /* __HAVE_BYZANZ_LAYER_WINDOW_H__ */
