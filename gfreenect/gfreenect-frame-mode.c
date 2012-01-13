/*
 * gfreenect-frame-mode.c
 *
 * gfreenect - A GObject wrapper of the libfreenect library
 * Copyright (C) 2011 Igalia S.L.
 *
 * Authors:
 *  Joaquim Manuel Pereira Rocha <jrocha@igalia.com>
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

#include <string.h>
#include <libfreenect.h>

#include "gfreenect-decls.h"
#include "gfreenect-frame-mode.h"

GType
gfreenect_frame_mode_get_type (void)
{
  static GType type = 0;

  if (G_UNLIKELY (type == 0))
    type = g_boxed_type_register_static ("GFreenectFrameMode",
                                         gfreenect_frame_mode_copy,
                                         gfreenect_frame_mode_free);
  return type;
}

/**
 * gfreenect_frame_mode_copy:
 *
 * Returns: (transfer full):
 **/
gpointer
gfreenect_frame_mode_copy (gpointer boxed)
{
  GFreenectFrameMode *mode;

  mode = g_slice_new0 (GFreenectFrameMode);

  memcpy (mode, boxed, sizeof (GFreenectFrameMode));

  return mode;
}

void
gfreenect_frame_mode_free (gpointer boxed)
{
  g_slice_free (GFreenectFrameMode, boxed);
}

/**
 * gfreenect_frame_mode_new_from_native:
 *
 * Returns: (transfer full) (type GFreenectFrameMode):
 **/
GFreenectFrameMode *
gfreenect_frame_mode_new_from_native (gpointer native)
{
  GFreenectFrameMode *mode;

  mode = g_slice_new0 (GFreenectFrameMode);

  gfreenect_frame_mode_set_from_native (mode, native);

  return mode;
}

void
gfreenect_frame_mode_set_from_native (GFreenectFrameMode *mode, gpointer native)
{
  freenect_frame_mode *_mode = native;

  mode->resolution = _mode->resolution;
  mode->video_format = _mode->video_format;
  mode->depth_format = _mode->depth_format;

  mode->length = _mode->bytes;
  mode->width = _mode->width;
  mode->height = _mode->height;
  mode->bits_per_pixel = _mode->data_bits_per_pixel;
  mode->padding_bits_per_pixel = _mode->padding_bits_per_pixel;

  mode->framerate = _mode->framerate;
}
