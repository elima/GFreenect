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

/**
 * SECTION:gfreenect-frame-mode
 * @short_description: Data structure holding meta-information about a camera
 * frame (depth, video, IR, etc).
 *
 * A #GFreenectFrameMode structure is built automatically by #GFreenectDevice
 * and provided to the user through the methods to obtain the streaming frames.
 * #GFreenectDevice holds information useful for describing and interpreting
 * a particular data frame, like @width and @height, @bits_per_pixel, etc.
 *
 * Use gfreenect_frame_mode_copy() to create an exact copy of the object and
 * gfreenect_frame_mode_free() to free it.
 *
 * gfreenect_frame_mode_new_from_native() and
 * gfreenect_frame_mode_set_from_native() are used internally by
 * #GFreenectDevice and should not normally be called in user code.
 **/

#include <string.h>
#include <libfreenect.h>

#include "gfreenect-decls.h"
#include "gfreenect-frame-mode.h"

/**
 * gfreenect_frame_mode_get_type:
 *
 * Returns: The registered #GType for #GFreenectFrameMode boxed type
 **/
GType
gfreenect_frame_mode_get_type (void)
{
  static GType type = 0;

  if (G_UNLIKELY (type == 0))
    type = g_boxed_type_register_static ("GFreenectFrameMode",
                                    (GBoxedCopyFunc) gfreenect_frame_mode_copy,
                                    (GBoxedFreeFunc) gfreenect_frame_mode_free);
  return type;
}

/**
 * gfreenect_frame_mode_copy:
 * @frame_mode: The #GFreenectFrameMode to copy
 *
 * Makes an exact copy of a #GFreenectFrameMode object.
 *
 * Returns: (transfer full): A newly created #GFreenectFrameMode. Use
 * gfreenect_frame_mode_free() to free it.
 **/
gpointer
gfreenect_frame_mode_copy (GFreenectFrameMode *frame_mode)
{
  GFreenectFrameMode *mode;

  mode = g_slice_new0 (GFreenectFrameMode);

  memcpy (mode, frame_mode, sizeof (GFreenectFrameMode));

  return mode;
}

/**
 * gfreenect_frame_mode_free:
 * @frame_mode: The #GFreenectFrameMode to free
 *
 * Frees a #GFreenectFrameMode object.
 **/
void
gfreenect_frame_mode_free (GFreenectFrameMode *frame_mode)
{
  g_slice_free (GFreenectFrameMode, frame_mode);
}

/**
 * gfreenect_frame_mode_new_from_native:
 * @native: Pointer to a #freenect_frame_mode structure
 *
 * Creates a new #GFreenectFrameMode structure and fills all its values using
 * information from a native #freenect_frame_mode structure. This is a low
 * level method that a user would rarely use.
 *
 * Returns: (transfer full): A newly created #GFreenectFrameMode. Use
 * gfreenect_frame_mode_free() to free it.
 **/
GFreenectFrameMode *
gfreenect_frame_mode_new_from_native (gpointer native)
{
  GFreenectFrameMode *mode;

  mode = g_slice_new0 (GFreenectFrameMode);

  gfreenect_frame_mode_set_from_native (mode, native);

  return mode;
}

/**
 * gfreenect_frame_mode_set_from_native:
 * @mode: A #GFreenectFrameMode object
 * @native: Pointer to a #freenect_frame_mode structure
 *
 * Sets all values of a #GFreenectFrameMode using the information contained
 * in a native #freenect_frame_mode structure. This is a low level method that
 * a user would rarely use.
 **/
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

  mode->frame_rate = _mode->framerate;
}
