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

#include "byzanzencodergif.h"

#include <string.h>
#include <glib/gi18n.h>

#include "gifenc.h"

G_DEFINE_TYPE (ByzanzEncoderGif, byzanz_encoder_gif, BYZANZ_TYPE_ENCODER)

static gboolean
byzanz_encoder_write_data (gpointer       closure,
                           const guchar * data,
                           gsize          len,
                           GError **      error)
{
  ByzanzEncoder *encoder = closure;

  return g_output_stream_write_all (encoder->output_stream, data, len,
      NULL, encoder->cancellable, error);
}

static gboolean
byzanz_encoder_gif_setup (ByzanzEncoder * encoder,
                          GOutputStream * stream,
                          guint           width,
                          guint           height,
                          GCancellable *  cancellable,
                          GError **	  error)
{
  ByzanzEncoderGif *gif = BYZANZ_ENCODER_GIF (encoder);

  gif->gifenc = gifenc_new (width, height, byzanz_encoder_write_data, encoder, NULL);

  gif->image_data = g_malloc (width * height);
  gif->cached_data = g_malloc (width * height);
  gif->cached_tmp = g_malloc (width * height);
  return TRUE;
}

static gboolean
byzanz_encoder_gif_quantize (ByzanzEncoderGif * gif,
                             cairo_surface_t *  surface,
                             GError **          error)
{
  GifencPalette *palette;

  g_assert (!gif->has_quantized);

  palette = gifenc_quantize_image (cairo_image_surface_get_data (surface),
      cairo_image_surface_get_width (surface), cairo_image_surface_get_height (surface),
      cairo_image_surface_get_stride (surface), TRUE, 255);
  
  if (!gifenc_initialize (gif->gifenc, palette, TRUE, error))
    return FALSE;

  memset (gif->image_data,
      gifenc_palette_get_alpha_index (palette),
      gifenc_get_width (gif->gifenc) * gifenc_get_height (gif->gifenc));

  gif->has_quantized = TRUE;
  return TRUE;
}

static gboolean
byzanz_encoder_write_image (ByzanzEncoderGif *gif, guint64 msecs, GError **error)
{
  guint elapsed;
  guint width;

  g_assert (gif->cached_data != NULL);
  g_assert (gif->cached_area.width > 0);
  g_assert (gif->cached_area.height > 0);

  width = gifenc_get_width (gif->gifenc);
  elapsed = msecs - gif->cached_time;
  elapsed = MAX (elapsed, 10);

  if (!gifenc_add_image (gif->gifenc, gif->cached_area.x, gif->cached_area.y, 
            gif->cached_area.width, gif->cached_area.height, elapsed,
            gif->cached_data + width * gif->cached_area.y + gif->cached_area.x,
            width, error))
    return FALSE;

  gif->cached_time = msecs;
  return TRUE;
}

static gboolean
byzanz_encoder_gif_encode_image (ByzanzEncoderGif *      gif,
                                 cairo_surface_t *       surface,
                                 const cairo_region_t *  region,
                                 cairo_rectangle_int_t * area_out)
{
  cairo_rectangle_int_t extents, area, rect;
  guint8 transparent;
  guint i, n_rects, stride, width;

  cairo_region_get_extents (region, &extents);
  transparent = gifenc_palette_get_alpha_index (gif->gifenc->palette);
  stride = cairo_image_surface_get_stride (surface);
  width = gifenc_get_width (gif->gifenc);

  /* clear area */
  /* FIXME: only do this in parts not captured by region */
  for (i = extents.y; i < (guint) (extents.y + extents.height); i++) {
    memset (gif->cached_tmp + width * i + extents.x, transparent, extents.width);
  }

  /* render changed parts */
  n_rects = cairo_region_num_rectangles (region);
  memset (area_out, 0, sizeof (cairo_rectangle_int_t));
  for (i = 0; i < n_rects; i++) {
    cairo_region_get_rectangle (region, i, &rect);
    if (gifenc_dither_rgb_with_full_image (
          gif->cached_tmp + width * rect.y + rect.x, width,
	  gif->image_data + width * rect.y + rect.x, width, 
	  gif->gifenc->palette, 
          cairo_image_surface_get_data (surface) + (rect.x - extents.x) * 4
              + (rect.y - extents.y) * stride,
          rect.width, rect.height, stride, &area)) {
      area.x += rect.x;
      area.y += rect.y;
      if (area_out->width > 0 && area_out->height > 0)
        gdk_rectangle_union ((const GdkRectangle*)area_out, (const GdkRectangle*) &area, (GdkRectangle*)area_out);
      else
        *area_out = area;
    }
  }

  return area_out->width > 0 && area_out->height > 0;
}

static void
byzanz_encoder_swap_image (ByzanzEncoderGif *      gif,
                           cairo_rectangle_int_t * area)
{
  guint8 *swap;

  swap = gif->cached_data;
  gif->cached_data = gif->cached_tmp;
  gif->cached_tmp = swap;
  gif->cached_area = *area;
}

static gboolean
byzanz_encoder_gif_process (ByzanzEncoder *        encoder,
                            GOutputStream *        stream,
                            guint64                msecs,
                            cairo_surface_t *      surface,
                            const cairo_region_t * region,
                            GCancellable *         cancellable,
                            GError **	           error)
{
  ByzanzEncoderGif *gif = BYZANZ_ENCODER_GIF (encoder);
  cairo_rectangle_int_t area;

  if (!gif->has_quantized) {
    if (!byzanz_encoder_gif_quantize (gif, surface, error))
      return FALSE;
    gif->cached_time = msecs;
    if (!byzanz_encoder_gif_encode_image (gif, surface, region, &area)) {
      g_assert_not_reached ();
    }
    byzanz_encoder_swap_image (gif, &area);
  } else {
    if (byzanz_encoder_gif_encode_image (gif, surface, region, &area)) {
      if (!byzanz_encoder_write_image (gif, msecs, error))
        return FALSE;
      byzanz_encoder_swap_image (gif, &area);
    }
  }

  return TRUE;
}

static gboolean
byzanz_encoder_gif_close (ByzanzEncoder *  encoder,
                          GOutputStream *  stream,
                          guint64          msecs,
                          GCancellable *   cancellable,
                          GError **	   error)
{
  ByzanzEncoderGif *gif = BYZANZ_ENCODER_GIF (encoder);

  if (!gif->has_quantized) {
    g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_FAILED, _("No image to encode."));
    return FALSE;
  }

  if (!byzanz_encoder_write_image (gif, msecs, error) ||
      !gifenc_close (gif->gifenc, error))
    return FALSE;

  return TRUE;
}

static void
byzanz_encoder_gif_finalize (GObject *object)
{
  ByzanzEncoderGif *gif = BYZANZ_ENCODER_GIF (object);

  g_free (gif->image_data);
  g_free (gif->cached_data);
  g_free (gif->cached_tmp);
  if (gif->gifenc)
    gifenc_free (gif->gifenc);

  G_OBJECT_CLASS (byzanz_encoder_gif_parent_class)->finalize (object);
}

static void
byzanz_encoder_gif_class_init (ByzanzEncoderGifClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ByzanzEncoderClass *encoder_class = BYZANZ_ENCODER_CLASS (klass);

  object_class->finalize = byzanz_encoder_gif_finalize;

  encoder_class->setup = byzanz_encoder_gif_setup;
  encoder_class->process = byzanz_encoder_gif_process;
  encoder_class->close = byzanz_encoder_gif_close;

  encoder_class->filter = gtk_file_filter_new ();
  g_object_ref_sink (encoder_class->filter);
  gtk_file_filter_set_name (encoder_class->filter, _("GIF images"));
  gtk_file_filter_add_mime_type (encoder_class->filter, "image/gif");
  gtk_file_filter_add_pattern (encoder_class->filter, "*.gif");
}

static void
byzanz_encoder_gif_init (ByzanzEncoderGif *encoder_gif)
{
}

