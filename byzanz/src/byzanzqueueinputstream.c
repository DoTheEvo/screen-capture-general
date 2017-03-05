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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "byzanzqueueinputstream.h"

G_DEFINE_TYPE (ByzanzQueueInputStream, byzanz_queue_input_stream, G_TYPE_INPUT_STREAM)

static gboolean
byzanz_queue_input_stream_close_input (ByzanzQueueInputStream *stream,
				       GCancellable *	       cancellable,
				       GError **               error)
{
  if (stream->input == NULL)
    return TRUE;

  if (!g_input_stream_close (stream->input, cancellable, error))
    return FALSE;

  g_object_unref (stream->input);
  stream->input = NULL;
  stream->input_bytes = 0;
  return TRUE;
}

static void
byzanz_queue_input_stream_dispose (GObject *object)
{
  ByzanzQueueInputStream *stream = BYZANZ_QUEUE_INPUT_STREAM (object);

  if (g_atomic_int_dec_and_test (&stream->queue->shared_count)) {
    stream->queue->input = NULL;
    g_object_unref (stream->queue);
  } else {
    g_object_ref (stream);
    return;
  }

  G_OBJECT_CLASS (byzanz_queue_input_stream_parent_class)->dispose (object);
}

static void
byzanz_queue_input_stream_finalize (GObject *object)
{
  ByzanzQueueInputStream *stream = BYZANZ_QUEUE_INPUT_STREAM (object);

  if (!byzanz_queue_input_stream_close_input (stream, NULL, NULL))
    g_object_unref (stream->input);

  G_OBJECT_CLASS (byzanz_queue_input_stream_parent_class)->finalize (object);
}

static gboolean
byzanz_queue_input_stream_wait (ByzanzQueueInputStream *stream,
			        GCancellable *		cancellable,
				GError **		error)
{
  GPollFD fd;
  guint n_fds;
  
  /* FIXME: Use a file monitor here */

  /* Do the same thing that the UNIX tail program does: sleep a second */
  n_fds = 0;
  if (cancellable)
    {
      g_cancellable_make_pollfd (cancellable, &fd);
      n_fds++;
    }

  g_poll (&fd, n_fds, 1000);

  return !g_cancellable_set_error_if_cancelled (cancellable, error);
}

static gboolean
byzanz_queue_input_stream_ensure_input (ByzanzQueueInputStream *stream,
					GCancellable *          cancellable,
					GError **               error)
{
  GFile *file;

  if (stream->input_bytes >= BYZANZ_QUEUE_FILE_SIZE)
    {
      if (!byzanz_queue_input_stream_close_input (stream, cancellable, error))
	return FALSE;
      stream->input = NULL;
    }

  if (stream->input != NULL)
    return TRUE;

  g_async_queue_lock (stream->queue->files);
  do {
    file = g_async_queue_try_pop_unlocked (stream->queue->files);
    if (file != NULL || stream->queue->output_closed)
      break;
    
    g_async_queue_unlock (stream->queue->files);
    if (!byzanz_queue_input_stream_wait (stream, cancellable, error))
      return FALSE;
    g_async_queue_lock (stream->queue->files);
  
  } while (TRUE);
  g_async_queue_unlock (stream->queue->files);

  if (file == NULL)
    return TRUE;

  stream->input = G_INPUT_STREAM (g_file_read (file, cancellable, error));
  g_file_delete (file, NULL, NULL);
  g_object_unref (file);

  return stream->input != NULL;
}

static gssize
byzanz_queue_input_stream_read (GInputStream *input_stream,
				void *	      buffer,
				gsize         count,
				GCancellable *cancellable,
				GError **     error)
{
  ByzanzQueueInputStream *stream = BYZANZ_QUEUE_INPUT_STREAM (input_stream);
  gssize result;

retry:
  if (!byzanz_queue_input_stream_ensure_input (stream, cancellable, error))
    return -1;

  /* No more data to read from the queue */
  if (stream->input == NULL)
    return 0;

  result = g_input_stream_read (stream->input, buffer, count, cancellable, error);
  if (result == -1)
    return -1;

  /* no data in file. Let's wait for more. */
  if (result == 0) {
    if (!byzanz_queue_input_stream_wait (stream, cancellable, error))
      return -1;
    goto retry;
  }

  stream->input_bytes += result;
  return result;
}

static gssize
byzanz_queue_input_stream_skip (GInputStream *input_stream,
				gsize         count,
				GCancellable *cancellable,
				GError **     error)
{
  ByzanzQueueInputStream *stream = BYZANZ_QUEUE_INPUT_STREAM (input_stream);
  gssize result;

retry:
  if (!byzanz_queue_input_stream_ensure_input (stream, cancellable, error))
    return -1;

  /* No more data to read from the queue */
  if (stream->input == NULL)
    return 0;

  result = g_input_stream_skip (stream->input, count, cancellable, error);
  if (result == -1)
    return -1;

  /* no data in file. Let's wait for more. */
  if (result == 0) {
    if (!byzanz_queue_input_stream_wait (stream, cancellable, error))
      return -1;
    goto retry;
  }

  stream->input_bytes += result;
  return result;
}

static gboolean
byzanz_queue_input_stream_close (GInputStream * input_stream,
				 GCancellable * cancellable,
				 GError **      error)
{
  ByzanzQueueInputStream *stream = BYZANZ_QUEUE_INPUT_STREAM (input_stream);
  GFile *file;

  if (!byzanz_queue_input_stream_close_input (stream, cancellable, error))
    return FALSE;

  g_async_queue_lock (stream->queue->files);
  stream->queue->input_closed = TRUE;
  file = g_async_queue_try_pop_unlocked (stream->queue->files);
  g_async_queue_unlock (stream->queue->files);

  while (file) {
    g_file_delete (file, NULL, NULL);
    file = g_async_queue_try_pop (stream->queue->files);
  }

  return TRUE;
}

static void
byzanz_queue_input_stream_class_init (ByzanzQueueInputStreamClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GInputStreamClass *input_stream_class = G_INPUT_STREAM_CLASS (klass);

  object_class->dispose = byzanz_queue_input_stream_dispose;
  object_class->finalize = byzanz_queue_input_stream_finalize;

  input_stream_class->read_fn = byzanz_queue_input_stream_read;
  input_stream_class->skip = byzanz_queue_input_stream_skip;
  input_stream_class->close_fn = byzanz_queue_input_stream_close;
  /* FIXME: implement async ops */
}

static void
byzanz_queue_input_stream_init (ByzanzQueueInputStream *queue_input_stream)
{
}

GInputStream *
byzanz_queue_input_stream_new (ByzanzQueue *queue)
{
  ByzanzQueueInputStream *stream;

  g_return_val_if_fail (BYZANZ_IS_QUEUE (queue), NULL);

  stream = g_object_new (BYZANZ_TYPE_QUEUE_INPUT_STREAM, NULL);
  stream->queue = queue;

  return G_INPUT_STREAM (stream);
}

