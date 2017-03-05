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

#include <glib.h>
#include <gtk/gtk.h>

#include "byzanzencoder.h"
#include "byzanzqueue.h"
#include "byzanzrecorder.h"

#ifndef __HAVE_BYZANZ_SESSION_H__
#define __HAVE_BYZANZ_SESSION_H__

typedef struct _ByzanzSession ByzanzSession;
typedef struct _ByzanzSessionClass ByzanzSessionClass;

#define BYZANZ_TYPE_SESSION                    (byzanz_session_get_type())
#define BYZANZ_IS_SESSION(obj)                 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BYZANZ_TYPE_SESSION))
#define BYZANZ_IS_SESSION_CLASS(klass)         (G_TYPE_CHECK_CLASS_TYPE ((klass), BYZANZ_TYPE_SESSION))
#define BYZANZ_SESSION(obj)                    (G_TYPE_CHECK_INSTANCE_CAST ((obj), BYZANZ_TYPE_SESSION, ByzanzSession))
#define BYZANZ_SESSION_CLASS(klass)            (G_TYPE_CHECK_CLASS_CAST ((klass), BYZANZ_TYPE_SESSION, ByzanzSessionClass))
#define BYZANZ_SESSION_GET_CLASS(obj)          (G_TYPE_INSTANCE_GET_CLASS ((obj), BYZANZ_TYPE_SESSION, ByzanzSessionClass))

struct _ByzanzSession {
  GObject		object;
  
  /*< private >*/
  /* properties */
  GFile *               file;           /* file we're saving to */
  cairo_rectangle_int_t area;           /* area of window to record */
  GdkWindow *           window;         /* window to record */
  gboolean              record_audio;   /* TRUE to record audio */
  GType                 encoder_type;   /* type of encoder to use */
  ByzanzQueue *         queue;          /* queue we use as data cache */
  GTimeVal              start_time;     /* when we started writing to queue */

  /* internal objects */
  GCancellable *        cancellable;    /* cancellable to use for aborting the session */
  ByzanzRecorder *      recorder;       /* the recorder in use */
  ByzanzEncoder *	encoder;	/* encoding thread */
  GError *              error;          /* NULL or the error we're in */
};

struct _ByzanzSessionClass {
  GObjectClass		object_class;
};

GType		        byzanz_session_get_type		(void) G_GNUC_CONST;


ByzanzSession * 	byzanz_session_new		(GFile *                        file,
                                                         GType                          encoder_type,
							 GdkWindow *		        window,
							 const cairo_rectangle_int_t *	area,
							 gboolean		        record_cursor,
                                                         gboolean                       record_audio);
void			byzanz_session_start		(ByzanzSession *	session);
void			byzanz_session_stop		(ByzanzSession *	session);
void			byzanz_session_abort            (ByzanzSession *	session);

gboolean                byzanz_session_is_recording     (ByzanzSession *        session);
gboolean                byzanz_session_is_encoding      (ByzanzSession *        session);
const GError *          byzanz_session_get_error        (ByzanzSession *        session);
					

#endif /* __HAVE_BYZANZ_SESSION_H__ */
