/*
 * gfreenect-frame-mode.h
 *
 * gfreenect - A GObject wrapper of the libfreenect library
 * Copyright (C) 2011 Igalia S.L.
 *
 * Authors:
 *  Eduardo Lima Mitev <elima@igalia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License at http://www.gnu.org/licenses/lgpl-3.0.txt
 * for more details.
 */

#ifndef __GFREENECT_FRAME_MODE_H__
#define __GFREENECT_FRAME_MODE_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define GFREENECT_TYPE_FRAME_MODE (gfreenect_frame_mode_get_type ())

typedef struct _GFreenectFrameMode GFreenectFrameMode;

/**
 * GFreenectFrameMode:
 * @resolution: The image resolution of the frame from #GFreenectResolution
 * @video_format: The video format of the frame from #GFreenectVideoFormat
 * @depth_format: The depth format of the frame from #GFreenectDepthFormat
 * @length: The length of the frame data, in bytes
 * @width: The width of the frame, in pixels
 * @height: The height of the frame, in pixels
 * @bits_per_pixel: The number of bits to represent the data of one pixel
 * @padding_bits_per_pixel: The number of padding bits in the data of one pixel
 * @frame_rate: The expected frame rate
 **/
struct _GFreenectFrameMode
{
  GFreenectResolution resolution;
  guint video_format;
  guint depth_format;
  gsize length;

  gsize width;
  gsize height;

  guint bits_per_pixel;
  guint padding_bits_per_pixel;

  guint frame_rate;
};

GType                gfreenect_frame_mode_get_type                (void);
gpointer             gfreenect_frame_mode_copy                    (GFreenectFrameMode *frame_mode);
void                 gfreenect_frame_mode_free                    (GFreenectFrameMode *frame_mode);

GFreenectFrameMode * gfreenect_frame_mode_new_from_native         (gpointer native);
void                 gfreenect_frame_mode_set_from_native         (GFreenectFrameMode *mode,
                                                                   gpointer            native);

G_END_DECLS

#endif /* __GFREENECT_FRAME_MODE_H__ */
