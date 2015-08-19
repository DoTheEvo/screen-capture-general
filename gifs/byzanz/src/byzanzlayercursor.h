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

#ifndef __HAVE_BYZANZ_LAYER_CURSOR_H__
#define __HAVE_BYZANZ_LAYER_CURSOR_H__

typedef struct _ByzanzLayerCursor ByzanzLayerCursor;
typedef struct _ByzanzLayerCursorClass ByzanzLayerCursorClass;

#define BYZANZ_TYPE_LAYER_CURSOR                    (byzanz_layer_cursor_get_type())
#define BYZANZ_IS_LAYER_CURSOR(obj)                 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BYZANZ_TYPE_LAYER_CURSOR))
#define BYZANZ_IS_LAYER_CURSOR_CLASS(klass)         (G_TYPE_CHECK_CLASS_TYPE ((klass), BYZANZ_TYPE_LAYER_CURSOR))
#define BYZANZ_LAYER_CURSOR(obj)                    (G_TYPE_CHECK_INSTANCE_CAST ((obj), BYZANZ_TYPE_LAYER_CURSOR, ByzanzLayerCursor))
#define BYZANZ_LAYER_CURSOR_CLASS(klass)            (G_TYPE_CHECK_CLASS_CAST ((klass), BYZANZ_TYPE_LAYER_CURSOR, ByzanzLayerCursorClass))
#define BYZANZ_LAYER_CURSOR_GET_CLASS(obj)          (G_TYPE_INSTANCE_GET_CLASS ((obj), BYZANZ_TYPE_LAYER_CURSOR, ByzanzLayerCursorClass))

struct _ByzanzLayerCursor {
  ByzanzLayer           layer;

  cairo_surface_t *     cursor_next;            /* current active cursor */
  cairo_surface_t *     cursor;                 /* last recorded cursor */
  int                   cursor_x;               /* last recorded X position of cursor */
  int                   cursor_y;               /* last recorded Y position of cursor */

  GHashTable *          cursors;                /* all cursors we know about already */

  guint                 poll_source;            /* source used for querying mouse position */
};

struct _ByzanzLayerCursorClass {
  ByzanzLayerClass	layer_class;
};

GType		        byzanz_layer_cursor_get_type	        (void) G_GNUC_CONST;


#endif /* __HAVE_BYZANZ_LAYER_CURSOR_H__ */
