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

#include "byzanzencoderbyzanz.h"

#include <string.h>
#include <glib/gi18n.h>

#include "byzanzserialize.h"

G_DEFINE_TYPE (ByzanzEncoderByzanz, byzanz_encoder_byzanz, BYZANZ_TYPE_ENCODER)

static gboolean
byzanz_encoder_byzanz_setup (ByzanzEncoder * encoder,
                             GOutputStream * stream,
                             guint           width,
                             guint           height,
                             GCancellable *  cancellable,
                             GError **	     error)
{
  return byzanz_serialize_header (stream, width, height, cancellable, error);
}

static gboolean
byzanz_encoder_byzanz_process (ByzanzEncoder *        encoder,
                               GOutputStream *        stream,
                               guint64                msecs,
                               cairo_surface_t *      surface,
                               const cairo_region_t * region,
                               GCancellable *         cancellable,
                               GError **	      error)
{
  return byzanz_serialize (stream, msecs, surface, region, cancellable, error);
}

static gboolean
byzanz_encoder_byzanz_close (ByzanzEncoder *  encoder,
                             GOutputStream *  stream,
                             guint64          msecs,
                             GCancellable *   cancellable,
                             GError **	      error)
{
  return byzanz_serialize (stream, msecs, NULL, NULL, cancellable, error);
}

static void
byzanz_encoder_byzanz_class_init (ByzanzEncoderByzanzClass *klass)
{
  ByzanzEncoderClass *encoder_class = BYZANZ_ENCODER_CLASS (klass);

  /* We don't use the run vfunc and just g_output_stream_slice() here,
   * because this way we get data verification.
   */
  encoder_class->setup = byzanz_encoder_byzanz_setup;
  encoder_class->process = byzanz_encoder_byzanz_process;
  encoder_class->close = byzanz_encoder_byzanz_close;

  encoder_class->filter = gtk_file_filter_new ();
  g_object_ref_sink (encoder_class->filter);
  gtk_file_filter_set_name (encoder_class->filter, _("Byzanz debug files"));
  gtk_file_filter_add_mime_type (encoder_class->filter, "application/x-byzanz");
  gtk_file_filter_add_pattern (encoder_class->filter, "*.byzanz");
}

static void
byzanz_encoder_byzanz_init (ByzanzEncoderByzanz *encoder_byzanz)
{
}

