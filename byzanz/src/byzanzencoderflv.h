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

#include "byzanzencodergstreamer.h"

#ifndef __HAVE_BYZANZ_ENCODER_FLV_H__
#define __HAVE_BYZANZ_ENCODER_FLV_H__

typedef struct _ByzanzEncoderFlv ByzanzEncoderFlv;
typedef struct _ByzanzEncoderFlvClass ByzanzEncoderFlvClass;

#define BYZANZ_TYPE_ENCODER_FLV                    (byzanz_encoder_flv_get_type())
#define BYZANZ_IS_ENCODER_FLV(obj)                 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BYZANZ_TYPE_ENCODER_FLV))
#define BYZANZ_IS_ENCODER_FLV_CLASS(klass)         (G_TYPE_CHECK_CLASS_TYPE ((klass), BYZANZ_TYPE_ENCODER_FLV))
#define BYZANZ_ENCODER_FLV(obj)                    (G_TYPE_CHECK_INSTANCE_CAST ((obj), BYZANZ_TYPE_ENCODER_FLV, ByzanzEncoderFlv))
#define BYZANZ_ENCODER_FLV_CLASS(klass)            (G_TYPE_CHECK_CLASS_CAST ((klass), BYZANZ_TYPE_ENCODER_FLV, ByzanzEncoderFlvClass))
#define BYZANZ_ENCODER_FLV_GET_CLASS(obj)          (G_TYPE_INSTANCE_GET_CLASS ((obj), BYZANZ_TYPE_ENCODER_FLV, ByzanzEncoderFlvClass))

struct _ByzanzEncoderFlv {
  ByzanzEncoderGStreamer        encoder;
};

struct _ByzanzEncoderFlvClass {
  ByzanzEncoderGStreamerClass   encoder_class;
};

GType		byzanz_encoder_flv_get_type		(void) G_GNUC_CONST;


#endif /* __HAVE_BYZANZ_ENCODER_FLV_H__ */
