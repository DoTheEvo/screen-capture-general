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
#include "gifenc.h"

#ifndef __HAVE_BYZANZ_ENCODER_GIF_H__
#define __HAVE_BYZANZ_ENCODER_GIF_H__

typedef struct _ByzanzEncoderGif ByzanzEncoderGif;
typedef struct _ByzanzEncoderGifClass ByzanzEncoderGifClass;

#define BYZANZ_TYPE_ENCODER_GIF                    (byzanz_encoder_gif_get_type())
#define BYZANZ_IS_ENCODER_GIF(obj)                 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BYZANZ_TYPE_ENCODER_GIF))
#define BYZANZ_IS_ENCODER_GIF_CLASS(klass)         (G_TYPE_CHECK_CLASS_TYPE ((klass), BYZANZ_TYPE_ENCODER_GIF))
#define BYZANZ_ENCODER_GIF(obj)                    (G_TYPE_CHECK_INSTANCE_CAST ((obj), BYZANZ_TYPE_ENCODER_GIF, ByzanzEncoderGif))
#define BYZANZ_ENCODER_GIF_CLASS(klass)            (G_TYPE_CHECK_CLASS_CAST ((klass), BYZANZ_TYPE_ENCODER_GIF, ByzanzEncoderGifClass))
#define BYZANZ_ENCODER_GIF_GET_CLASS(obj)          (G_TYPE_INSTANCE_GET_CLASS ((obj), BYZANZ_TYPE_ENCODER_GIF, ByzanzEncoderGifClass))

struct _ByzanzEncoderGif {
  ByzanzEncoder         encoder;

  Gifenc *		gifenc;		/* encoder used to encode the image */

  gboolean              has_quantized;  /* qantization has happened already */
  guint8 *              image_data;     /* width * height of encoded image */

  cairo_rectangle_int_t cached_area;    /* area that is saved in cached_data */
  guint8 *              cached_data;    /* cached_area.{width x height} sized area of image */
  guint64               cached_time;    /* timestamp the cached image corresponds to */

  guint8 *		cached_tmp;	/* temporary data to swap cached_data with */
};

struct _ByzanzEncoderGifClass {
  ByzanzEncoderClass    encoder_class;
};

GType		byzanz_encoder_gif_get_type		(void) G_GNUC_CONST;


#endif /* __HAVE_BYZANZ_ENCODER_GIF_H__ */
