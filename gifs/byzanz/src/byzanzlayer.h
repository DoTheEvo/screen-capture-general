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

#include <gdk/gdk.h>
#include "byzanzrecorder.h"

#ifndef __HAVE_BYZANZ_LAYER_H__
#define __HAVE_BYZANZ_LAYER_H__

typedef struct _ByzanzLayer ByzanzLayer;
typedef struct _ByzanzLayerClass ByzanzLayerClass;

#define BYZANZ_TYPE_LAYER                    (byzanz_layer_get_type())
#define BYZANZ_IS_LAYER(obj)                 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BYZANZ_TYPE_LAYER))
#define BYZANZ_IS_LAYER_CLASS(klass)         (G_TYPE_CHECK_CLASS_TYPE ((klass), BYZANZ_TYPE_LAYER))
#define BYZANZ_LAYER(obj)                    (G_TYPE_CHECK_INSTANCE_CAST ((obj), BYZANZ_TYPE_LAYER, ByzanzLayer))
#define BYZANZ_LAYER_CLASS(klass)            (G_TYPE_CHECK_CLASS_CAST ((klass), BYZANZ_TYPE_LAYER, ByzanzLayerClass))
#define BYZANZ_LAYER_GET_CLASS(obj)          (G_TYPE_INSTANCE_GET_CLASS ((obj), BYZANZ_TYPE_LAYER, ByzanzLayerClass))

struct _ByzanzLayer {
  GObject		object;

  /*< protected >*/
  ByzanzRecorder *      recorder;               /* the recorder we are recording from (not keeping a reference to it) */
};

struct _ByzanzLayerClass {
  GObjectClass		object_class;

  gboolean              (* event)                       (ByzanzLayer *          layer,
                                                         GdkXEvent *            event);
  cairo_region_t *      (* snapshot)                    (ByzanzLayer *          layer);
  void                  (* render)                      (ByzanzLayer *          layer,
                                                         cairo_t *              cr);
};

GType		        byzanz_layer_get_type	        (void) G_GNUC_CONST;


/* protected API for subclasses */
void                    byzanz_layer_invalidate         (ByzanzLayer *          layer);


#endif /* __HAVE_BYZANZ_LAYER_H__ */
