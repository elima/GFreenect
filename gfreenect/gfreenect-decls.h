/*
 * gfreenect-decls.h
 *
 * gfreenect - A GObject wrapper of the libfreenect library
 * Copyright (C) 2011 Igalia S.L.
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

/**
 * SECTION: gfreenect-decls
 * @short_description: Global symbols
 *
 * Global symbols used throughout the library.
 **/

#ifndef __GFREENECT_DECLS_H__
#define __GFREENECT_DECLS_H__

#include <glib.h>

/**
 * GFreenectSubdevice:
 * @GFREENECT_SUBDEVICE_MOTOR: Motor subdevice
 * @GFREENECT_SUBDEVICE_CAMERA: Camera subdevice
 * @GFREENECT_SUBDEVICE_AUDIO: Audio subdevice
 * @GFREENECT_SUBDEVICE_ALL: Combination of all subdevices
 *
 * Enumeration of available subdevices. Used in gfreenect_device_new()
 * to specify what subdevices to activate.
 **/
typedef enum
{
  GFREENECT_SUBDEVICE_MOTOR  = 0x01,
  GFREENECT_SUBDEVICE_CAMERA = 0x02,
  GFREENECT_SUBDEVICE_AUDIO  = 0x04,
  GFREENECT_SUBDEVICE_ALL    = GFREENECT_SUBDEVICE_MOTOR |
                               GFREENECT_SUBDEVICE_CAMERA |
                               GFREENECT_SUBDEVICE_AUDIO
} GFreenectSubdevice;

/**
 * GFreenectResolution:
 * @GFREENECT_RESOLUTION_LOW: Low resolution (QVGA - 320x240)
 * @GFREENECT_RESOLUTION_MEDIUM: Medium resolution (VGA  - 640x480)
 * @GFREENECT_RESOLUTION_HIGH: High resolution (SXGA - 1280x1024)
 *
 * Available resolutions for depth and video camera streams.
 **/
typedef enum {
  GFREENECT_RESOLUTION_LOW    = 0,
  GFREENECT_RESOLUTION_MEDIUM = 1,
  GFREENECT_RESOLUTION_HIGH   = 2
} GFreenectResolution;

/**
 * GFreenectDepthFormat:
 * @GFREENECT_DEPTH_FORMAT_11BIT: 11 bit depth information in one uint16 per
 * pixel
 * @GFREENECT_DEPTH_FORMAT_10BIT: 10 bit depth information in one uint16 per
 * pixel
 * @GFREENECT_DEPTH_FORMAT_11BIT_PACKED: 11 bit packed depth information
 * @GFREENECT_DEPTH_FORMAT_10BIT_PACKED: 10 bit packed depth information
 * @GFREENECT_DEPTH_FORMAT_REGISTERED: Processed depth data in mm, aligned to 640x480 RGB
 * @GFREENECT_DEPTH_FORMAT_MM: Depth to each pixel in mm, but left unaligned to RGB image
 *
 * Available formats for the depth camera stream.
 **/
typedef enum {
  GFREENECT_DEPTH_FORMAT_11BIT         = 0,
  GFREENECT_DEPTH_FORMAT_10BIT         = 1,
  GFREENECT_DEPTH_FORMAT_11BIT_PACKED  = 2,
  GFREENECT_DEPTH_FORMAT_10BIT_PACKED  = 3,
  GFREENECT_DEPTH_FORMAT_REGISTERED    = 4,
  GFREENECT_DEPTH_FORMAT_MM            = 5
} GFreenectDepthFormat;

/**
 * GFreenectVideoFormat:
 * @GFREENECT_VIDEO_FORMAT_RGB: Decompressed RGB mode (demosaicing done by
 * libfreenect)
 * @GFREENECT_VIDEO_FORMAT_BAYER: Bayer compressed mode (raw information from
 * camera)
 * @GFREENECT_VIDEO_FORMAT_IR_8BIT: 8-bit IR mode
 * @GFREENECT_VIDEO_FORMAT_IR_10BIT: 10-bit IR mode
 * @GFREENECT_VIDEO_FORMAT_IR_10BIT_PACKED: 10-bit packed IR mode
 * @GFREENECT_VIDEO_FORMAT_YUV_RGB: YUV RGB mode
 * @GFREENECT_VIDEO_FORMAT_YUV_RAW: YUV raw mode
 *
 * Available video formats for the video camera stream.
 **/
typedef enum {
  GFREENECT_VIDEO_FORMAT_RGB             = 0,
  GFREENECT_VIDEO_FORMAT_BAYER           = 1,
  GFREENECT_VIDEO_FORMAT_IR_8BIT         = 2,
  GFREENECT_VIDEO_FORMAT_IR_10BIT        = 3,
  GFREENECT_VIDEO_FORMAT_IR_10BIT_PACKED = 4,
  GFREENECT_VIDEO_FORMAT_YUV_RGB         = 5,
  GFREENECT_VIDEO_FORMAT_YUV_RAW         = 6
} GFreenectVideoFormat;

/**
 * GFreenectLed:
 * @GFREENECT_LED_OFF: LED off
 * @GFREENECT_LED_GREEN: LED green
 * @GFREENECT_LED_RED: LED red
 * @GFREENECT_LED_YELLOW: LED yellow
 * @GFREENECT_LED_BLINK_GREEN: LED blinking green
 * @GFREENECT_LED_BLINK_RED_YELLOW: LED blinking red/yellow
 *
 * Available LED states.
 **/
typedef enum {
  GFREENECT_LED_OFF              = 0,
  GFREENECT_LED_GREEN            = 1,
  GFREENECT_LED_RED              = 2,
  GFREENECT_LED_YELLOW           = 3,
  GFREENECT_LED_BLINK_GREEN      = 4,
  GFREENECT_LED_BLINK_RED_YELLOW = 6
} GFreenectLed;

#endif /* __GFREENECT_DECLS_H__ */
