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

#include "byzanzencoderogv.h"

#include <glib/gi18n.h>

G_DEFINE_TYPE (ByzanzEncoderOgv, byzanz_encoder_ogv, BYZANZ_TYPE_ENCODER_GSTREAMER)

static void
byzanz_encoder_ogv_class_init (ByzanzEncoderOgvClass *klass)
{
  ByzanzEncoderClass *encoder_class = BYZANZ_ENCODER_CLASS (klass);
  ByzanzEncoderGStreamerClass *gstreamer_class = BYZANZ_ENCODER_GSTREAMER_CLASS (klass);

  encoder_class->filter = gtk_file_filter_new ();
  g_object_ref_sink (encoder_class->filter);
  gtk_file_filter_set_name (encoder_class->filter, _("Theora video"));
  gtk_file_filter_add_mime_type (encoder_class->filter, "video/ogg");
  gtk_file_filter_add_pattern (encoder_class->filter, "*.ogv");
  gtk_file_filter_add_pattern (encoder_class->filter, "*.ogg");

  gstreamer_class->pipeline_string = 
    "appsrc name=src ! videoconvert ! videorate !"
    "video/x-raw,format=Y444,framerate=25/1 ! theoraenc ! oggmux ! giostreamsink name=sink";
  gstreamer_class->audio_pipeline_string = 
    "autoaudiosrc name=audiosrc ! audioconvert ! queue ! oggmux name=muxer ! giostreamsink name=sink "
    "appsrc name=src ! videoconvert ! videorate ! "
    "video/x-raw,format=Y444,framerate=25/1 ! theoraenc ! queue ! muxer.";
}

static void
byzanz_encoder_ogv_init (ByzanzEncoderOgv *encoder_ogv)
{
}

