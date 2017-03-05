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

#include "byzanzencoderflv.h"

#include <glib/gi18n.h>

G_DEFINE_TYPE (ByzanzEncoderFlv, byzanz_encoder_flv, BYZANZ_TYPE_ENCODER_GSTREAMER)

static void
byzanz_encoder_flv_class_init (ByzanzEncoderFlvClass *klass)
{
  ByzanzEncoderClass *encoder_class = BYZANZ_ENCODER_CLASS (klass);
  ByzanzEncoderGStreamerClass *gstreamer_class = BYZANZ_ENCODER_GSTREAMER_CLASS (klass);

  encoder_class->filter = gtk_file_filter_new ();
  g_object_ref_sink (encoder_class->filter);
  gtk_file_filter_set_name (encoder_class->filter, _("Flash video"));
  gtk_file_filter_add_mime_type (encoder_class->filter, "video/x-flv");
  gtk_file_filter_add_pattern (encoder_class->filter, "*.flv");

  gstreamer_class->pipeline_string = 
    "appsrc name=src ! videoconvert ! avenc_flashsv buffer-size=8388608 ! flvmux ! giostreamsink name=sink";
  gstreamer_class->audio_pipeline_string = 
    "autoaudiosrc name=audiosrc ! audioconvert ! audio/x-raw-int,width=16 ! queue ! flvmux name=muxer ! giostreamsink name=sink "
    "appsrc name=src ! videoconvert ! avenc_flashsv buffer-size=8388608 ! muxer.";
}

static void
byzanz_encoder_flv_init (ByzanzEncoderFlv *encoder_flv)
{
}

