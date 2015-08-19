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

#ifndef __HAVE_BYZANZ_RECORDER_H__
#define __HAVE_BYZANZ_RECORDER_H__

typedef struct _ByzanzRecorder ByzanzRecorder;
typedef struct _ByzanzRecorderClass ByzanzRecorderClass;

/* 25 fps */
#define BYZANZ_RECORDER_FRAME_RATE_MS 1000 / 25

#define BYZANZ_TYPE_RECORDER                    (byzanz_recorder_get_type())
#define BYZANZ_IS_RECORDER(obj)                 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BYZANZ_TYPE_RECORDER))
#define BYZANZ_IS_RECORDER_CLASS(klass)         (G_TYPE_CHECK_CLASS_TYPE ((klass), BYZANZ_TYPE_RECORDER))
#define BYZANZ_RECORDER(obj)                    (G_TYPE_CHECK_INSTANCE_CAST ((obj), BYZANZ_TYPE_RECORDER, ByzanzRecorder))
#define BYZANZ_RECORDER_CLASS(klass)            (G_TYPE_CHECK_CLASS_CAST ((klass), BYZANZ_TYPE_RECORDER, ByzanzRecorderClass))
#define BYZANZ_RECORDER_GET_CLASS(obj)          (G_TYPE_INSTANCE_GET_CLASS ((obj), BYZANZ_TYPE_RECORDER, ByzanzRecorderClass))

struct _ByzanzRecorder {
  GObject		object;

  GdkWindow *           window;                 /* window we are recording from */
  cairo_rectangle_int_t area;                   /* area of window that we record */
  gboolean              recording;              /* wether we should be recording now */

  int                   damage_event_base;      /* base event for Damage extension */
  int                   damage_error_base;      /* base error for Damage extension */
  int                   fixes_event_base;       /* base event for Fixes extension */
  int                   fixes_error_base;       /* base error for Fixes extension */

  GSequence *           layers;                 /* sequence of ByzanzLayer, ordered by layer depth */

  guint                 next_image_source;      /* timer that fires when enough time after the last frame has elapsed */
};

struct _ByzanzRecorderClass {
  GObjectClass		object_class;

  void                  (* image)                       (ByzanzRecorder *        recorder,
                                                         cairo_surface_t *       surface,
                                                         const cairo_surface_t * region,
                                                         const GTimeVal *        tv);
};

GType		        byzanz_recorder_get_type	(void) G_GNUC_CONST;

ByzanzRecorder *	byzanz_recorder_new		(GdkWindow *		 window,
							 cairo_rectangle_int_t * area);

void                    byzanz_recorder_set_recording   (ByzanzRecorder *       recorder,
                                                         gboolean               recording);
gboolean                byzanz_recorder_get_recording   (ByzanzRecorder *       recorder);

void                    byzanz_recorder_queue_snapshot  (ByzanzRecorder *       recorder);


#endif /* __HAVE_BYZANZ_RECORDER_H__ */
