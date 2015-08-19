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

#include "byzanzqueue.h"

#ifndef __HAVE_BYZANZ_QUEUE_OUTPUT_STREAM_H__
#define __HAVE_BYZANZ_QUEUE_OUTPUT_STREAM_H__

typedef struct _ByzanzQueueOutputStream ByzanzQueueOutputStream;
typedef struct _ByzanzQueueOutputStreamClass ByzanzQueueOutputStreamClass;

#define BYZANZ_TYPE_QUEUE_OUTPUT_STREAM                    (byzanz_queue_output_stream_get_type())
#define BYZANZ_IS_QUEUE_OUTPUT_STREAM(obj)                 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BYZANZ_TYPE_QUEUE_OUTPUT_STREAM))
#define BYZANZ_IS_QUEUE_OUTPUT_STREAM_CLASS(klass)         (G_TYPE_CHECK_CLASS_TYPE ((klass), BYZANZ_TYPE_QUEUE_OUTPUT_STREAM))
#define BYZANZ_QUEUE_OUTPUT_STREAM(obj)                    (G_TYPE_CHECK_INSTANCE_CAST ((obj), BYZANZ_TYPE_QUEUE_OUTPUT_STREAM, ByzanzQueueOutputStream))
#define BYZANZ_QUEUE_OUTPUT_STREAM_CLASS(klass)            (G_TYPE_CHECK_CLASS_CAST ((klass), BYZANZ_TYPE_QUEUE_OUTPUT_STREAM, ByzanzQueueOutputStreamClass))
#define BYZANZ_QUEUE_OUTPUT_STREAM_GET_CLASS(obj)          (G_TYPE_INSTANCE_GET_CLASS ((obj), BYZANZ_TYPE_QUEUE_OUTPUT_STREAM, ByzanzQueueOutputStreamClass))

struct _ByzanzQueueOutputStream {
  GOutputStream		output_stream;

  ByzanzQueue *		queue;		/* queue we belong to */
  GOutputStream *	output;		/* stream we're writing to or %NULL if we need to open one */
  goffset		output_bytes;	/* bytes we may still write to output */
};

struct _ByzanzQueueOutputStreamClass {
  GOutputStreamClass	output_stream_class;
};

GType		byzanz_queue_output_stream_get_type		(void) G_GNUC_CONST;

GOutputStream *	byzanz_queue_output_stream_new			(ByzanzQueue *	queue);


#endif /* __HAVE_BYZANZ_QUEUE_OUTPUT_STREAM_H__ */
