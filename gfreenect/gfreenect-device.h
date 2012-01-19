/*
 * gfreenect-device.h
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

#ifndef __GFREENECT_DEVICE_H__
#define __GFREENECT_DEVICE_H__

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include <gfreenect-decls.h>

#include <gfreenect-frame-mode.h>

G_BEGIN_DECLS

typedef struct _GFreenectDevice GFreenectDevice;
typedef struct _GFreenectDeviceClass GFreenectDeviceClass;
typedef struct _GFreenectDevicePrivate GFreenectDevicePrivate;

struct _GFreenectDevice
{
  GObject parent;

  GFreenectDevicePrivate *priv;
};

/**
 * GFreenectDeviceClass:
 * @depth_frame: Prototype for #GFreenectDevice::depth-frame signal
 * @video_frame: Prototype for #GFreenectDevice::video-frame signal
 **/
struct _GFreenectDeviceClass
{
  GObjectClass parent_class;

  /* Signal prototypes */
  void (* depth_frame) (GFreenectDevice *self, gpointer user_data);
  void (* video_frame) (GFreenectDevice *self, gpointer user_data);
};

#define GFREENECT_TYPE_DEVICE           (gfreenect_device_get_type ())
#define GFREENECT_DEVICE(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), GFREENECT_TYPE_DEVICE, GFreenectDevice))
#define GFREENECT_DEVICE_CLASS(obj)     (G_TYPE_CHECK_CLASS_CAST ((obj), GFREENECT_TYPE_DEVICE, GFreenectDeviceClass))
#define GFREENECT_IS_DEVICE(obj)        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GFREENECT_TYPE_DEVICE))
#define GFREENECT_IS_DEVICE_CLASS(obj)  (G_TYPE_CHECK_CLASS_TYPE ((obj), GFREENECT_TYPE_DEVICE))
#define GFREENECT_DEVICE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GFREENECT_TYPE_DEVICE, GFreenectDeviceClass))


GType             gfreenect_device_get_type                   (void) G_GNUC_CONST;

void              gfreenect_device_new                        (gint                 device_index,
                                                               guint                subdevices,
                                                               GCancellable        *cancellable,
                                                               GAsyncReadyCallback  callback,
                                                               gpointer             user_data);
GFreenectDevice * gfreenect_device_new_finish                 (GAsyncResult  *result,
                                                               GError       **error);

gboolean          gfreenect_device_start_depth_stream         (GFreenectDevice       *self,
                                                               GFreenectDepthFormat   format,
                                                               GError               **error);
gboolean          gfreenect_device_start_video_stream         (GFreenectDevice      *self,
                                                               GFreenectResolution   resolution,
                                                               GFreenectVideoFormat  format,
                                                               GError               **error);

gboolean          gfreenect_device_stop_depth_stream          (GFreenectDevice  *self,
                                                               GError          **error);
gboolean          gfreenect_device_stop_video_stream          (GFreenectDevice  *self,
                                                               GError          **error);

void              gfreenect_device_set_led                    (GFreenectDevice     *self,
                                                               GFreenectLed         led,
                                                               GCancellable        *cancellable,
                                                               GAsyncReadyCallback  callback,
                                                               gpointer             user_data);
gboolean          gfreenect_device_set_led_finish             (GFreenectDevice  *self,
                                                               GAsyncResult     *result,
                                                               GError          **error);

guint8 *          gfreenect_device_get_depth_frame_raw        (GFreenectDevice    *self,
                                                               gsize              *len,
                                                               GFreenectFrameMode *frame_mode);
guint8 *          gfreenect_device_get_video_frame_raw        (GFreenectDevice    *self,
                                                               gsize              *len,
                                                               GFreenectFrameMode *frame_mode);

guint8 *          gfreenect_device_get_depth_frame_grayscale  (GFreenectDevice    *self,
                                                               gsize              *len,
                                                               GFreenectFrameMode *frame_mode);

guint8 *          gfreenect_device_get_video_frame_rgb        (GFreenectDevice    *self,
                                                               gsize              *len,
                                                               GFreenectFrameMode *frame_mode);

void              gfreenect_device_set_tilt_angle             (GFreenectDevice     *self,
                                                               gdouble              tilt_angle,
                                                               GCancellable        *cancellable,
                                                               GAsyncReadyCallback  callback,
                                                               gpointer             user_data);
gboolean          gfreenect_device_set_tilt_angle_finish      (GFreenectDevice  *self,
                                                               GAsyncResult     *result,
                                                               GError          **error);

void              gfreenect_device_get_tilt_angle             (GFreenectDevice     *self,
                                                               GCancellable        *cancellable,
                                                               GAsyncReadyCallback  callback,
                                                               gpointer             user_data);
gdouble           gfreenect_device_get_tilt_angle_finish      (GFreenectDevice  *self,
                                                               GAsyncResult     *result,
                                                               GError          **error);

gdouble           gfreenect_device_get_tilt_angle_sync        (GFreenectDevice  *self,
                                                               GCancellable     *cancellable,
                                                               GError          **error);

void              gfreenect_device_get_accel                  (GFreenectDevice     *self,
                                                               GCancellable        *cancellable,
                                                               GAsyncReadyCallback  callback,
                                                               gpointer             user_data);
gboolean          gfreenect_device_get_accel_finish           (GFreenectDevice  *self,
                                                               gdouble          *x,
                                                               gdouble          *y,
                                                               gdouble          *z,
                                                               GAsyncResult     *result,
                                                               GError          **error);

gboolean          gfreenect_device_get_accel_sync             (GFreenectDevice  *self,
                                                               gdouble          *x,
                                                               gdouble          *y,
                                                               gdouble          *z,
                                                               GCancellable     *cancellable,
                                                               GError          **error);

G_END_DECLS

#endif /* __GFREENECT_DEVICE_H__ */
