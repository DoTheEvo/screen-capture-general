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

#include "byzanzlayer.h"

#include <gdk/gdkx.h>

enum {
  PROP_0,
  PROP_RECORDER,
};

G_DEFINE_ABSTRACT_TYPE (ByzanzLayer, byzanz_layer, G_TYPE_OBJECT)

static void
byzanz_layer_set_property (GObject *object, guint param_id, const GValue *value, 
    GParamSpec * pspec)
{
  ByzanzLayer *layer = BYZANZ_LAYER (object);

  switch (param_id) {
    case PROP_RECORDER:
      layer->recorder = g_value_get_object (value);
      g_assert (layer->recorder != NULL);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
  }
}

static void
byzanz_layer_get_property (GObject *object, guint param_id, GValue *value, 
    GParamSpec * pspec)
{
  ByzanzLayer *layer = BYZANZ_LAYER (object);

  switch (param_id) {
    case PROP_RECORDER:
      g_value_set_object (value, layer->recorder);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
  }
}

static void
byzanz_layer_constructed (GObject *object)
{
  ByzanzLayer *layer = BYZANZ_LAYER (object);

  byzanz_layer_invalidate (layer);

  if (G_OBJECT_CLASS (byzanz_layer_parent_class)->constructed)
    G_OBJECT_CLASS (byzanz_layer_parent_class)->constructed (object);
}

static void
byzanz_layer_finalize (GObject *object)
{
  //ByzanzLayer *layer = BYZANZ_LAYER (object);

  G_OBJECT_CLASS (byzanz_layer_parent_class)->finalize (object);
}

static void
byzanz_layer_class_init (ByzanzLayerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = byzanz_layer_get_property;
  object_class->set_property = byzanz_layer_set_property;
  object_class->finalize = byzanz_layer_finalize;
  object_class->constructed = byzanz_layer_constructed;

  g_object_class_install_property (object_class, PROP_RECORDER,
      g_param_spec_object ("recorder", "recorder", "recorder that manages us",
	  BYZANZ_TYPE_RECORDER, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
}

static void
byzanz_layer_init (ByzanzLayer *layer)
{
}

void
byzanz_layer_invalidate (ByzanzLayer *layer)
{
  g_return_if_fail (BYZANZ_IS_LAYER (layer));

  byzanz_recorder_queue_snapshot (layer->recorder);
}

