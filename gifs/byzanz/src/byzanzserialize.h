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

#include <gio/gio.h>
#include <gdk/gdk.h>
#include <cairo.h>

#ifndef __HAVE_BYZANZ_SERIALIZE_H__
#define __HAVE_BYZANZ_SERIALIZE_H__


gboolean                byzanz_serialize_header         (GOutputStream *        stream,
                                                         guint                  width,
                                                         guint                  height,
                                                         GCancellable *         cancellable,
                                                         GError **              error);
gboolean                byzanz_serialize                (GOutputStream *         stream,
                                                         guint64                 msecs,
                                                         cairo_surface_t *       surface,
                                                         const cairo_region_t * region,
                                                         GCancellable *          cancellable,
                                                         GError **               error);

gboolean                byzanz_deserialize_header       (GInputStream *         stream,
                                                         guint *                width,
                                                         guint *                height,
                                                         GCancellable *         cancellable,
                                                         GError **              error);
gboolean                byzanz_deserialize              (GInputStream *         stream,
                                                         guint64 *              msecs_out,
                                                         cairo_surface_t **     surface_out,
                                                         cairo_region_t **      region_out,
                                                         GCancellable *         cancellable,
                                                         GError **              error);


#endif /* __HAVE_BYZANZ_SERIALIZE_H__ */
