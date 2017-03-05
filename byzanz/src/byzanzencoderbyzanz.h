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

#ifndef __HAVE_BYZANZ_ENCODER_BYZANZ_H__
#define __HAVE_BYZANZ_ENCODER_BYZANZ_H__

typedef struct _ByzanzEncoderByzanz ByzanzEncoderByzanz;
typedef struct _ByzanzEncoderByzanzClass ByzanzEncoderByzanzClass;

#define BYZANZ_TYPE_ENCODER_BYZANZ                    (byzanz_encoder_byzanz_get_type())
#define BYZANZ_IS_ENCODER_BYZANZ(obj)                 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BYZANZ_TYPE_ENCODER_BYZANZ))
#define BYZANZ_IS_ENCODER_BYZANZ_CLASS(klass)         (G_TYPE_CHECK_CLASS_TYPE ((klass), BYZANZ_TYPE_ENCODER_BYZANZ))
#define BYZANZ_ENCODER_BYZANZ(obj)                    (G_TYPE_CHECK_INSTANCE_CAST ((obj), BYZANZ_TYPE_ENCODER_BYZANZ, ByzanzEncoderByzanz))
#define BYZANZ_ENCODER_BYZANZ_CLASS(klass)            (G_TYPE_CHECK_CLASS_CAST ((klass), BYZANZ_TYPE_ENCODER_BYZANZ, ByzanzEncoderByzanzClass))
#define BYZANZ_ENCODER_BYZANZ_GET_CLASS(obj)          (G_TYPE_INSTANCE_GET_CLASS ((obj), BYZANZ_TYPE_ENCODER_BYZANZ, ByzanzEncoderByzanzClass))

struct _ByzanzEncoderByzanz {
  ByzanzEncoder         encoder;
};

struct _ByzanzEncoderByzanzClass {
  ByzanzEncoderClass    encoder_class;
};

GType		byzanz_encoder_byzanz_get_type		(void) G_GNUC_CONST;


#endif /* __HAVE_BYZANZ_ENCODER_BYZANZ_H__ */
