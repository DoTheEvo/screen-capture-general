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

#include "byzanzqueueoutputstream.h"

#include <unistd.h>

G_DEFINE_TYPE (ByzanzQueueOutputStream, byzanz_queue_output_stream, G_TYPE_OUTPUT_STREAM)

static void
byzanz_queue_output_stream_dispose (GObject *object)
{
  ByzanzQueueOutputStream *stream = BYZANZ_QUEUE_OUTPUT_STREAM (object);

  if (g_atomic_int_dec_and_test (&stream->queue->shared_count)) {
    stream->queue->output = NULL;
    g_object_unref (stream->queue);
  } else {
    g_object_ref (stream);
    return;
  }

  G_OBJECT_CLASS (byzanz_queue_output_stream_parent_class)->dispose (object);
}

static void
byzanz_queue_output_stream_finalize (GObject *object)
{
  ByzanzQueueOutputStream *stream = BYZANZ_QUEUE_OUTPUT_STREAM (object);

  if (stream->output)
    g_object_unref (stream->output);

  G_OBJECT_CLASS (byzanz_queue_output_stream_parent_class)->finalize (object);
}

static gboolean
byzanz_queue_output_stream_ensure_output (ByzanzQueueOutputStream *stream,
					  GCancellable *           cancellable,
					  GError **                error)
{
  GFile *file;
  int fd;
  char *filename;

  if (stream->output_bytes == 0 && stream->output)
    {
      if (!g_output_stream_close (stream->output, cancellable, error))
	return FALSE;
      g_object_unref (stream->output);
      stream->output = NULL;
    }

  if (stream->output != NULL)
    return TRUE;

  g_async_queue_lock (stream->queue->files);

  if (stream->queue->input_closed) {
    g_async_queue_unlock (stream->queue->files);
    return TRUE;
  }

  fd = g_file_open_tmp ("byzanzcacheXXXXXX", &filename, error);
  if (fd < 0) {
    file = NULL;
  } else {
    close (fd);
    file = g_file_new_for_path (filename);
    g_free (filename);
    g_object_ref (file);
    g_async_queue_push_unlocked (stream->queue->files, file);
  }

  g_async_queue_unlock (stream->queue->files);

  if (file == NULL)
    return FALSE;

  stream->output = G_OUTPUT_STREAM (g_file_append_to (file, G_FILE_CREATE_PRIVATE, cancellable, error));
  g_object_unref (file);
  if (stream->output == NULL)
    return FALSE;

  stream->output_bytes = BYZANZ_QUEUE_FILE_SIZE;
  return TRUE;
}

static gssize
byzanz_queue_output_stream_write (GOutputStream *output_stream,
				  const void *   buffer,
				  gsize          count,
				  GCancellable * cancellable,
				  GError **      error)
{
  ByzanzQueueOutputStream *stream = BYZANZ_QUEUE_OUTPUT_STREAM (output_stream);
  gssize result;

  if (!byzanz_queue_output_stream_ensure_output (stream, cancellable, error))
    return -1;

  /* will happen if input stream is closed, and there's no need to continue writing */
  if (stream->output == NULL)
    return count;

  result = g_output_stream_write (stream->output, buffer, 
      MIN ((goffset) count, stream->output_bytes), cancellable, error);
  if (result == -1)
    return -1;

  stream->output_bytes -= result;
  return result;
}

static gboolean
byzanz_queue_output_stream_close (GOutputStream *output_stream,
                                  GCancellable * cancellable,
                                  GError **	 error)
{
  ByzanzQueueOutputStream *stream = BYZANZ_QUEUE_OUTPUT_STREAM (output_stream);

  if (stream->output &&
      !g_output_stream_close (stream->output, cancellable, error))
    return FALSE;

  g_async_queue_lock (stream->queue->files);
  stream->queue->output_closed = TRUE;
  g_async_queue_unlock (stream->queue->files);
  return TRUE;
}

static void
byzanz_queue_output_stream_class_init (ByzanzQueueOutputStreamClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GOutputStreamClass *output_stream_class = G_OUTPUT_STREAM_CLASS (klass);

  object_class->dispose = byzanz_queue_output_stream_dispose;
  object_class->finalize = byzanz_queue_output_stream_finalize;

  output_stream_class->write_fn = byzanz_queue_output_stream_write;
  output_stream_class->close_fn = byzanz_queue_output_stream_close;
  /* FIXME: implement async ops */
}

static void
byzanz_queue_output_stream_init (ByzanzQueueOutputStream *queue_output_stream)
{
}

GOutputStream *
byzanz_queue_output_stream_new (ByzanzQueue *queue)
{
  ByzanzQueueOutputStream *stream;

  g_return_val_if_fail (BYZANZ_IS_QUEUE (queue), NULL);

  stream = g_object_new (BYZANZ_TYPE_QUEUE_OUTPUT_STREAM, NULL);
  stream->queue = queue;

  return G_OUTPUT_STREAM (stream);
}

