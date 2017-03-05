/* desktop session recorder
 * Copyright (C) 2010 Benjamin Otte <otte@gnome.org>
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

#ifndef __HAVE_BYZANZ_ENCODER_WEBM_H__
#define __HAVE_BYZANZ_ENCODER_WEBM_H__

typedef struct _ByzanzEncoderWebm ByzanzEncoderWebm;
typedef struct _ByzanzEncoderWebmClass ByzanzEncoderWebmClass;

#define BYZANZ_TYPE_ENCODER_WEBM                    (byzanz_encoder_webm_get_type())
#define BYZANZ_IS_ENCODER_WEBM(obj)                 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BYZANZ_TYPE_ENCODER_WEBM))
#define BYZANZ_IS_ENCODER_WEBM_CLASS(klass)         (G_TYPE_CHECK_CLASS_TYPE ((klass), BYZANZ_TYPE_ENCODER_WEBM))
#define BYZANZ_ENCODER_WEBM(obj)                    (G_TYPE_CHECK_INSTANCE_CAST ((obj), BYZANZ_TYPE_ENCODER_WEBM, ByzanzEncoderWebm))
#define BYZANZ_ENCODER_WEBM_CLASS(klass)            (G_TYPE_CHECK_CLASS_CAST ((klass), BYZANZ_TYPE_ENCODER_WEBM, ByzanzEncoderWebmClass))
#define BYZANZ_ENCODER_WEBM_GET_CLASS(obj)          (G_TYPE_INSTANCE_GET_CLASS ((obj), BYZANZ_TYPE_ENCODER_WEBM, ByzanzEncoderWebmClass))

struct _ByzanzEncoderWebm {
  ByzanzEncoderGStreamer        encoder;
};

struct _ByzanzEncoderWebmClass {
  ByzanzEncoderGStreamerClass   encoder_class;
};

GType		byzanz_encoder_webm_get_type		(void) G_GNUC_CONST;


#endif /* __HAVE_BYZANZ_ENCODER_WEBM_H__ */
