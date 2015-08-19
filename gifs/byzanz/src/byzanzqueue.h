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

#ifndef __HAVE_BYZANZ_QUEUE_H__
#define __HAVE_BYZANZ_QUEUE_H__

typedef struct _ByzanzQueue ByzanzQueue;
typedef struct _ByzanzQueueClass ByzanzQueueClass;

#define BYZANZ_QUEUE_FILE_SIZE 16 * 1024 * 1024

#define BYZANZ_TYPE_QUEUE                    (byzanz_queue_get_type())
#define BYZANZ_IS_QUEUE(obj)                 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BYZANZ_TYPE_QUEUE))
#define BYZANZ_IS_QUEUE_CLASS(klass)         (G_TYPE_CHECK_CLASS_TYPE ((klass), BYZANZ_TYPE_QUEUE))
#define BYZANZ_QUEUE(obj)                    (G_TYPE_CHECK_INSTANCE_CAST ((obj), BYZANZ_TYPE_QUEUE, ByzanzQueue))
#define BYZANZ_QUEUE_CLASS(klass)            (G_TYPE_CHECK_CLASS_CAST ((klass), BYZANZ_TYPE_QUEUE, ByzanzQueueClass))
#define BYZANZ_QUEUE_GET_CLASS(obj)          (G_TYPE_INSTANCE_GET_CLASS ((obj), BYZANZ_TYPE_QUEUE, ByzanzQueueClass))

struct _ByzanzQueue {
  GObject		object;

  GOutputStream *	output;		/* stream to use for writing to the queue */
  GInputStream *	input;		/* stream to use for reading to the queue */

  volatile int		shared_count;	/* shared ref count of queue, output and input stream */

  GAsyncQueue *		files;		/* the files that still need to be processed */
  guint			output_closed:1;/* the output stream is closed. Must take async queue lock to access */
  guint			input_closed:1; /* the input stream is closed. Must take async queue lock to access */
};

struct _ByzanzQueueClass {
  GObjectClass		object_class;
};

GType		byzanz_queue_get_type		(void) G_GNUC_CONST;

ByzanzQueue *	byzanz_queue_new		(void);

GOutputStream *	byzanz_queue_get_output_stream	(ByzanzQueue *	queue);
GInputStream *	byzanz_queue_get_input_stream	(ByzanzQueue *	queue);


#endif /* __HAVE_BYZANZ_QUEUE_H__ */
