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

#ifndef __HAVE_BYZANZ_QUEUE_INPUT_STREAM_H__
#define __HAVE_BYZANZ_QUEUE_INPUT_STREAM_H__

typedef struct _ByzanzQueueInputStream ByzanzQueueInputStream;
typedef struct _ByzanzQueueInputStreamClass ByzanzQueueInputStreamClass;

#define BYZANZ_TYPE_QUEUE_INPUT_STREAM                    (byzanz_queue_input_stream_get_type())
#define BYZANZ_IS_QUEUE_INPUT_STREAM(obj)                 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BYZANZ_TYPE_QUEUE_INPUT_STREAM))
#define BYZANZ_IS_QUEUE_INPUT_STREAM_CLASS(klass)         (G_TYPE_CHECK_CLASS_TYPE ((klass), BYZANZ_TYPE_QUEUE_INPUT_STREAM))
#define BYZANZ_QUEUE_INPUT_STREAM(obj)                    (G_TYPE_CHECK_INSTANCE_CAST ((obj), BYZANZ_TYPE_QUEUE_INPUT_STREAM, ByzanzQueueInputStream))
#define BYZANZ_QUEUE_INPUT_STREAM_CLASS(klass)            (G_TYPE_CHECK_CLASS_CAST ((klass), BYZANZ_TYPE_QUEUE_INPUT_STREAM, ByzanzQueueInputStreamClass))
#define BYZANZ_QUEUE_INPUT_STREAM_GET_CLASS(obj)          (G_TYPE_INSTANCE_GET_CLASS ((obj), BYZANZ_TYPE_QUEUE_INPUT_STREAM, ByzanzQueueInputStreamClass))

struct _ByzanzQueueInputStream {
  GInputStream  	input_stream;

  ByzanzQueue *		queue;		/* queue we belong to */
  GInputStream *	input;		/* stream we're reading from or NULL if we need to open one */
  goffset		input_bytes;	/* bytes we've already read from input */
};

struct _ByzanzQueueInputStreamClass {
  GInputStreamClass	input_stream_class;
};

GType		byzanz_queue_input_stream_get_type		(void) G_GNUC_CONST;

GInputStream *	byzanz_queue_input_stream_new			(ByzanzQueue *	queue);


#endif /* __HAVE_BYZANZ_QUEUE_INPUT_STREAM_H__ */
