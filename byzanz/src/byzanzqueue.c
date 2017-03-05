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

#include "byzanzqueue.h"

#include "byzanzqueueinputstream.h"
#include "byzanzqueueoutputstream.h"

enum {
  PROP_0,
  PROP_INPUT,
  PROP_OUTPUT,
};

G_DEFINE_TYPE (ByzanzQueue, byzanz_queue, G_TYPE_OBJECT)

static void
byzanz_queue_get_property (GObject *object, guint param_id, GValue *value, 
    GParamSpec * pspec)
{
  ByzanzQueue *queue = BYZANZ_QUEUE (object);

  switch (param_id) {
    case PROP_INPUT:
      g_value_set_object (value, queue->input);
      break;
    case PROP_OUTPUT:
      g_value_set_object (value, queue->output);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
  }
}

static void
byzanz_queue_set_property (GObject *object, guint param_id, const GValue *value, 
    GParamSpec * pspec)
{
  //ByzanzQueue *queue = BYZANZ_QUEUE (object);

  switch (param_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
  }
}

static void
byzanz_queue_dispose (GObject *object)
{
  ByzanzQueue *queue = BYZANZ_QUEUE (object);

  if (queue->input)
    g_object_unref (queue->input);
  if (queue->output)
    g_object_unref (queue->output);

  if (!g_atomic_int_dec_and_test (&queue->shared_count)) {
    g_object_ref (queue);
    return;
  }

  G_OBJECT_CLASS (byzanz_queue_parent_class)->dispose (object);
}

static void
byzanz_queue_finalize (GObject *object)
{
  ByzanzQueue *queue = BYZANZ_QUEUE (object);
  GFile *file;

  while ((file = g_async_queue_try_pop (queue->files)))
    g_file_delete (file, NULL, NULL);
  g_async_queue_unref (queue->files);

  G_OBJECT_CLASS (byzanz_queue_parent_class)->dispose (object);
}

static void
byzanz_queue_class_init (ByzanzQueueClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = byzanz_queue_get_property;
  object_class->set_property = byzanz_queue_set_property;
  object_class->dispose = byzanz_queue_dispose;
  object_class->finalize = byzanz_queue_finalize;

  g_object_class_install_property (object_class, PROP_INPUT,
      g_param_spec_object ("inputstream", "input stream", "stream to use for reading from the cache",
	  G_TYPE_INPUT_STREAM, G_PARAM_READABLE));
  g_object_class_install_property (object_class, PROP_OUTPUT,
      g_param_spec_object ("outputstream", "output stream", "stream to use for writing to the cache",
	  G_TYPE_OUTPUT_STREAM, G_PARAM_READABLE));
}

static void
byzanz_queue_init (ByzanzQueue *queue)
{
  queue->files = g_async_queue_new ();

  queue->input = byzanz_queue_input_stream_new (queue);
  queue->output = byzanz_queue_output_stream_new (queue);

  queue->shared_count = 3;
}

ByzanzQueue *
byzanz_queue_new (void)
{
  return g_object_new (BYZANZ_TYPE_QUEUE, NULL);
}

GOutputStream *
byzanz_queue_get_output_stream (ByzanzQueue *queue)
{
  g_return_val_if_fail (BYZANZ_IS_QUEUE (queue), NULL);

  return queue->output;
}

GInputStream *
byzanz_queue_get_input_stream (ByzanzQueue *queue)
{
  g_return_val_if_fail (BYZANZ_IS_QUEUE (queue), NULL);

  return queue->input;
}

