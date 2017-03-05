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

#include <glib-object.h>
#include <gio/gio.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <cairo.h>

#ifndef __HAVE_BYZANZ_ENCODER_H__
#define __HAVE_BYZANZ_ENCODER_H__

typedef struct _ByzanzEncoder ByzanzEncoder;
typedef struct _ByzanzEncoderClass ByzanzEncoderClass;
typedef gpointer ByzanzEncoderIter;

#define BYZANZ_TYPE_ENCODER                    (byzanz_encoder_get_type())
#define BYZANZ_IS_ENCODER(obj)                 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BYZANZ_TYPE_ENCODER))
#define BYZANZ_IS_ENCODER_CLASS(klass)         (G_TYPE_CHECK_CLASS_TYPE ((klass), BYZANZ_TYPE_ENCODER))
#define BYZANZ_ENCODER(obj)                    (G_TYPE_CHECK_INSTANCE_CAST ((obj), BYZANZ_TYPE_ENCODER, ByzanzEncoder))
#define BYZANZ_ENCODER_CLASS(klass)            (G_TYPE_CHECK_CLASS_CAST ((klass), BYZANZ_TYPE_ENCODER, ByzanzEncoderClass))
#define BYZANZ_ENCODER_GET_CLASS(obj)          (G_TYPE_INSTANCE_GET_CLASS ((obj), BYZANZ_TYPE_ENCODER, ByzanzEncoderClass))

struct _ByzanzEncoder {
  GObject		object;
  
  /*<private >*/
  GInputStream *        input_stream;           /* stream to read from in byzanzserialize.h format */
  GOutputStream *       output_stream;          /* stream we write to (passed to the vfuncs) */
  gboolean              record_audio;           /* TRUE when we're recording audio */
  GCancellable *        cancellable;            /* cancellable to use in thread */
  GError *              error;                  /* NULL or the encoding error */

  GAsyncQueue *         jobs;                   /* the stuff we still need to encode */
  GThread *             thread;                 /* the encoding thread */
};

struct _ByzanzEncoderClass {
  GObjectClass		object_class;

  /*< protected >*/
  GtkFileFilter *       filter;                 /* filter to determine if a file should be encoded by this class */

  /* default function run in thread */
  gboolean              (* run)                 (ByzanzEncoder *        encoder,
                                                 GInputStream *         input,
                                                 GOutputStream *        output,
                                                 gboolean               record_audio,
                                                 GCancellable *         cancellable,
						 GError **		error);

  /* functions called by the default function */
  gboolean		(* setup)		(ByzanzEncoder *	encoder,
						 GOutputStream *	stream,
                                                 guint                  width,
                                                 guint                  height,
                                                 GCancellable *         cancellable,
						 GError **		error);
  gboolean		(* process)		(ByzanzEncoder *	encoder,
						 GOutputStream *	stream,
                                                 guint64                msecs,
						 cairo_surface_t *	surface,
						 const cairo_region_t *	region,
                                                 GCancellable *         cancellable,
						 GError **		error);
  gboolean		(* close)		(ByzanzEncoder *	encoder,
						 GOutputStream *	stream,
                                                 guint64                msecs,
                                                 GCancellable *         cancellable,
						 GError **		error);
};

GType		byzanz_encoder_get_type		(void) G_GNUC_CONST;

ByzanzEncoder *	byzanz_encoder_new		(GType                  encoder_type,
                                                 GInputStream *         input,
                                                 GOutputStream *        output,
                                                 gboolean               record_audio,
                                                 GCancellable *         cancellable);
/*
void		byzanz_encoder_process		(ByzanzEncoder *	encoder,
						 cairo_surface_t *	surface,
						 const cairo_region_t *	region,
						 const GTimeVal *	total_elapsed);
void		byzanz_encoder_close		(ByzanzEncoder *	encoder,
						 const GTimeVal *	total_elapsed);
*/
gboolean        byzanz_encoder_is_running       (ByzanzEncoder *        encoder);
const GError *  byzanz_encoder_get_error        (ByzanzEncoder *        encoder);

GtkFileFilter * byzanz_encoder_type_get_filter  (GType                  encoder_type);
GType           byzanz_encoder_get_type_from_filter 
                                                (GtkFileFilter *        filter);
GType           byzanz_encoder_get_type_from_file
                                                (GFile *                file);
GType           byzanz_encoder_type_iter_init   (ByzanzEncoderIter *    iter);
GType           byzanz_encoder_type_iter_next   (ByzanzEncoderIter *    iter);

#endif /* __HAVE_BYZANZ_ENCODER_H__ */
