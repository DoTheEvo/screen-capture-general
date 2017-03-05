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

#include "byzanzencoder.h"

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>

#ifndef __HAVE_BYZANZ_ENCODER_GSTREAMER_H__
#define __HAVE_BYZANZ_ENCODER_GSTREAMER_H__

typedef struct _ByzanzEncoderGStreamer ByzanzEncoderGStreamer;
typedef struct _ByzanzEncoderGStreamerClass ByzanzEncoderGStreamerClass;

#define BYZANZ_TYPE_ENCODER_GSTREAMER                    (byzanz_encoder_gstreamer_get_type())
#define BYZANZ_IS_ENCODER_GSTREAMER(obj)                 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BYZANZ_TYPE_ENCODER_GSTREAMER))
#define BYZANZ_IS_ENCODER_GSTREAMER_CLASS(klass)         (G_TYPE_CHECK_CLASS_TYPE ((klass), BYZANZ_TYPE_ENCODER_GSTREAMER))
#define BYZANZ_ENCODER_GSTREAMER(obj)                    (G_TYPE_CHECK_INSTANCE_CAST ((obj), BYZANZ_TYPE_ENCODER_GSTREAMER, ByzanzEncoderGStreamer))
#define BYZANZ_ENCODER_GSTREAMER_CLASS(klass)            (G_TYPE_CHECK_CLASS_CAST ((klass), BYZANZ_TYPE_ENCODER_GSTREAMER, ByzanzEncoderGStreamerClass))
#define BYZANZ_ENCODER_GSTREAMER_GET_CLASS(obj)          (G_TYPE_INSTANCE_GET_CLASS ((obj), BYZANZ_TYPE_ENCODER_GSTREAMER, ByzanzEncoderGStreamerClass))

struct _ByzanzEncoderGStreamer {
  ByzanzEncoder         encoder;

  cairo_surface_t *     surface;        /* last surface pushed down the pipeline */
  GTimeVal              start_time;     /* timestamp of first image */

  GstElement *          pipeline;       /* The pipeline */
  GstElement *          audiosrc;       /* the source we record audio from */
  GstAppSrc *           src;            /* the source we feed with images */
  GstCaps *             caps;           /* caps of video stream */
};

struct _ByzanzEncoderGStreamerClass {
  ByzanzEncoderClass    encoder_class;

  const char *          pipeline_string;
  const char *          audio_pipeline_string;
};

GType		byzanz_encoder_gstreamer_get_type		(void) G_GNUC_CONST;


#endif /* __HAVE_BYZANZ_ENCODER_GSTREAMER_H__ */
