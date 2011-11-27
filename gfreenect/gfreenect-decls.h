/*
 * gfreenect-decls.h
 *
 * gfreenect - A GObject wrapper of the libfreenect library
 * Copyright (C) 2011 Igalia S.L/
 *
 * Authors:
 *  Joaquim Manuel Pereira Rocha <jrocha@igalia.com>
 *  Eduardo Lima Mitev <elima@igalia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 3, or (at your option) any later version as published by
 * the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License at http://www.gnu.org/licenses/lgpl-3.0.txt
 * for more details.
 */

#ifndef __GFREENECT_DECLS_H__
#define __GFREENECT_DECLS_H__

typedef enum
{
  GFREENECT_SUBDEVICE_MOTOR  = 0x01,
  GFREENECT_SUBDEVICE_CAMERA = 0x02,
  GFREENECT_SUBDEVICE_AUDIO  = 0x04
} GFreenectSubdevice;

typedef enum {
  GFREENECT_RESOLUTION_LOW    = 0,
  GFREENECT_RESOLUTION_MEDIUM = 1,
  GFREENECT_RESOLUTION_HIGH   = 2,
  GFREENECT_RESOLUTION_DUMMY  = G_MAXUINT32
} GFreenectResolution;

typedef enum {
  GFREENECT_DEPTH_FORMAT_11BIT         = 0,
  GFREENECT_DEPTH_FORMAT_10BIT         = 1,
  GFREENECT_DEPTH_FORMAT_11BIT_PACKED  = 2,
  GFREENECT_DEPTH_FORMAT_10BIT_PACKED  = 3,
  GFREENECT_DEPTH_FORMAT_DUMMY         = G_MAXUINT32
} GFreenectDepthFormat;

typedef enum {
  GFREENECT_LED_OFF              = 0,
  GFREENECT_LED_GREEN            = 1,
  GFREENECT_LED_RED              = 2,
  GFREENECT_LED_YELLOW           = 3,
  GFREENECT_LED_BLINK_GREEN      = 4,
  GFREENECT_LED_BLINK_RED_YELLOW = 6
} GFreenectLed;

#endif /* __GFREENECT_DECLS_H__ */
