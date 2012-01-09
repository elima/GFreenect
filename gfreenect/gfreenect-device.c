/*
 * gfreenect-device.c
 *
 * gfreenect - A GObject wrapper of the libfreenect library
 * Copyright (C) 2011 Igalia S.L/
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

#include <libfreenect.h>
#include <string.h>
#include <math.h>

#include "gfreenect-device.h"

#define GFREENECT_DEVICE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
                                           GFREENECT_TYPE_DEVICE, \
                                           GFreenectDevicePrivate))

#define DEFAULT_SUBDEVICES       GFREENECT_SUBDEVICE_CAMERA | GFREENECT_SUBDEVICE_MOTOR
#define DEFAULT_DEPTH_FORMAT     GFREENECT_DEPTH_FORMAT_11BIT
#define DEFAULT_VIDEO_RESOLUTION GFREENECT_RESOLUTION_MEDIUM
#define DEFAULT_VIDEO_FORMAT     GFREENECT_VIDEO_FORMAT_RGB
#define DEFAULT_TILT_ANGLE       0.0

#define USER_BUF_SIZE 1280 * 1024 * 3

/* private data */
struct _GFreenectDevicePrivate
{
  GMainContext *glib_context;

  gint index;
  guint subdevices;
  GFreenectDepthFormat depth_format;
  GFreenectVideoFormat video_format;
  GFreenectResolution video_resolution;
  GFreenectLed led;
  gdouble tilt_angle;

  freenect_context *ctx;
  freenect_device *dev;

  freenect_frame_mode depth_mode;
  freenect_frame_mode video_mode;

  GThread *dispatch_thread;
  GMutex *mutex;
  gboolean abort_dispatch_thread;

  guint depth_frame_src_id;
  guint video_frame_src_id;

  void *user_buf;

  void *depth_buf;
  gboolean got_depth_frame;

  void *video_buf;
  gboolean got_video_frame;

  gboolean update_tilt_angle;
  gboolean update_led;
};

/* constructor data */
typedef struct
{
  GAsyncInitable *self;
  GSimpleAsyncResult *result;
  GCancellable *cancellable;
} ConstructorData;

/* signals */
enum
{
  SIGNAL_DEPTH_FRAME,
  SIGNAL_VIDEO_FRAME,
  LAST_SIGNAL
};

static guint gfreenect_device_signals [LAST_SIGNAL] = { 0 };

/* properties */
enum
{
  PROP_0,
  PROP_INDEX,
  PROP_SUBDEVICES,
  PROP_LED,
  PROP_TILT_ANGLE
};


static void     gfreenect_device_class_init         (GFreenectDeviceClass *class);
static void     gfreenect_device_init               (GFreenectDevice *self);
static void     gfreenect_device_finalize           (GObject *obj);
static void     gfreenect_device_dispose            (GObject *obj);

static void     gfreenect_device_set_property       (GObject *obj,
                                                     guint prop_id,
                                                     const GValue *value,
                                                     GParamSpec *pspec);
static void     gfreenect_device_get_property       (GObject *obj,
                                                     guint prop_id,
                                                     GValue *value,
                                                     GParamSpec *pspec);

static void     gfreenect_async_initable_iface_init (GAsyncInitableIface *iface);
static void     gfreenect_initable_iface_init       (GInitableIface *iface);

static void     init_async                          (GAsyncInitable      *initable,
                                                     int                  io_priority,
                                                     GCancellable        *cancellable,
                                                     GAsyncReadyCallback  callback,
                                                     gpointer             user_data);
static gboolean init_finish                         (GAsyncInitable  *initable,
                                                     GAsyncResult    *res,
                                                     GError         **error);
static gboolean init_sync                           (GInitable     *initable,
                                                     GCancellable  *cancellable,
                                                     GError       **error);

G_DEFINE_TYPE_WITH_CODE (GFreenectDevice, gfreenect_device, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (G_TYPE_ASYNC_INITABLE,
                                                gfreenect_async_initable_iface_init)
                         G_IMPLEMENT_INTERFACE (G_TYPE_INITABLE,
                                                gfreenect_initable_iface_init))

static void
gfreenect_device_class_init (GFreenectDeviceClass *class)
{
  GObjectClass *obj_class;

  obj_class = G_OBJECT_CLASS (class);

  obj_class->dispose = gfreenect_device_dispose;
  obj_class->finalize = gfreenect_device_finalize;
  obj_class->get_property = gfreenect_device_get_property;
  obj_class->set_property = gfreenect_device_set_property;

  /* install signals */
  gfreenect_device_signals[SIGNAL_DEPTH_FRAME] =
    g_signal_new ("depth-frame",
          G_TYPE_FROM_CLASS (obj_class),
          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
          G_STRUCT_OFFSET (GFreenectDeviceClass, depth_frame),
          NULL, NULL,
          g_cclosure_marshal_VOID__VOID,
          G_TYPE_NONE, 0);

  gfreenect_device_signals[SIGNAL_VIDEO_FRAME] =
    g_signal_new ("video-frame",
          G_TYPE_FROM_CLASS (obj_class),
          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
          G_STRUCT_OFFSET (GFreenectDeviceClass, video_frame),
          NULL, NULL,
          g_cclosure_marshal_VOID__VOID,
          G_TYPE_NONE, 0);

  /* install properties */
  g_object_class_install_property (obj_class,
                                   PROP_INDEX,
                                   g_param_spec_int ("index",
                                                     "Device index",
                                                     "The index of the device in the USB hub",
                                                     -1,
                                                     8,
                                                     -1,
                                                     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
                                                     G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (obj_class,
                                   PROP_SUBDEVICES,
                                   g_param_spec_uint ("subdevices",
                                                      "Subdevices",
                                                      "Subdevices to activate from GFreenectSubdevicesEnum",
                                                      0,
                                                      GFREENECT_SUBDEVICE_MOTOR | GFREENECT_SUBDEVICE_CAMERA | GFREENECT_SUBDEVICE_AUDIO,
                                                      DEFAULT_SUBDEVICES,
                                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
                                                      G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (obj_class,
                                   PROP_LED,
                                   g_param_spec_uint ("led",
                                                      "Led",
                                                      "Status of the led",
                                                      GFREENECT_LED_OFF,
                                                      GFREENECT_LED_BLINK_RED_YELLOW,
                                                      GFREENECT_LED_OFF,
                                                      G_PARAM_READWRITE |
                                                      G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (obj_class,
                                   PROP_TILT_ANGLE,
                                   g_param_spec_double ("tilt-angle",
                                                        "Tilt angle",
                                                        "Vertical angle relative to the horizon",
                                                        -30.0,
                                                        30.0,
                                                        DEFAULT_TILT_ANGLE,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_STRINGS));

  /* add private structure */
  g_type_class_add_private (obj_class, sizeof (GFreenectDevicePrivate));
}

static void
gfreenect_async_initable_iface_init (GAsyncInitableIface *iface)
{
  iface->init_async = init_async;
  iface->init_finish = init_finish;
}

static void
gfreenect_initable_iface_init (GInitableIface *iface)
{
  iface->init = init_sync;
}

static void
gfreenect_device_init (GFreenectDevice *self)
{
  GFreenectDevicePrivate *priv;

  priv = GFREENECT_DEVICE_GET_PRIVATE (self);
  self->priv = priv;

  priv->glib_context = NULL;

  priv->ctx = NULL;
  priv->dev = NULL;

  priv->dispatch_thread = NULL;
  priv->mutex = g_mutex_new ();

  priv->video_resolution = DEFAULT_VIDEO_RESOLUTION;
  priv->video_format = DEFAULT_VIDEO_FORMAT;
}

static void
gfreenect_device_dispose (GObject *obj)
{
  GFreenectDevice *self = GFREENECT_DEVICE (obj);

  g_mutex_lock (self->priv->mutex);

  if (self->priv->depth_frame_src_id != 0)
    {
      g_source_remove (self->priv->depth_frame_src_id);
      self->priv->depth_frame_src_id = 0;
    }

  if (self->priv->video_frame_src_id != 0)
    {
      g_source_remove (self->priv->video_frame_src_id);
      self->priv->video_frame_src_id = 0;
    }

  g_mutex_unlock (self->priv->mutex);

  if (self->priv->dispatch_thread != NULL)
    {
      self->priv->abort_dispatch_thread = TRUE;
      g_thread_join (self->priv->dispatch_thread);
    }

  g_mutex_free (self->priv->mutex);

  if (self->priv->dev != NULL)
    {
      freenect_close_device (self->priv->dev);
      self->priv->dev = NULL;
    }

  if (self->priv->ctx != NULL)
    {
      freenect_shutdown (self->priv->ctx);
      self->priv->ctx = NULL;
    }

  G_OBJECT_CLASS (gfreenect_device_parent_class)->dispose (obj);
}

static void
gfreenect_device_finalize (GObject *obj)
{
  GFreenectDevice *self = GFREENECT_DEVICE (obj);

  if (self->priv->depth_buf != NULL)
    {
      g_slice_free1 (self->priv->depth_mode.bytes, self->priv->depth_buf);
      self->priv->depth_buf = NULL;
    }

  if (self->priv->video_buf != NULL)
    {
      g_slice_free1 (self->priv->video_mode.bytes, self->priv->video_buf);
      self->priv->video_buf = NULL;
    }

  if (self->priv->user_buf != NULL)
    {
      g_slice_free1 (USER_BUF_SIZE, self->priv->user_buf);
      self->priv->user_buf = NULL;
    }

  G_OBJECT_CLASS (gfreenect_device_parent_class)->finalize (obj);
}

static void
gfreenect_device_set_property (GObject      *obj,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  GFreenectDevice *self;

  self = GFREENECT_DEVICE (obj);

  switch (prop_id)
    {
    case PROP_INDEX:
      self->priv->index = g_value_get_int (value);
      break;

    case PROP_SUBDEVICES:
      self->priv->subdevices = g_value_get_uint (value);
      break;

    case PROP_LED:
      gfreenect_device_set_led (self, g_value_get_uint (value));
      break;

    case PROP_TILT_ANGLE:
      gfreenect_device_set_tilt_angle (self, g_value_get_double (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
      break;
    }
}

static void
gfreenect_device_get_property (GObject    *obj,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  GFreenectDevice *self;

  self = GFREENECT_DEVICE (obj);

  switch (prop_id)
    {
    case PROP_INDEX:
      g_value_set_int (value, self->priv->index);
      break;

    case PROP_SUBDEVICES:
      g_value_set_uint (value, self->priv->subdevices);
      break;

    case PROP_LED:
      g_value_set_uint (value, self->priv->led);
      break;

    case PROP_TILT_ANGLE:
      g_value_set_double (value, self->priv->tilt_angle);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
      break;
    }
}

static guint
timeout_add (GMainContext *context,
             guint         timeout,
             gint          priority,
             GSourceFunc   callback,
             gpointer      user_data)
{
  guint src_id;
  GSource *src;

  if (context == NULL)
    context = g_main_context_get_thread_default ();

  if (timeout == 0)
    src = g_idle_source_new ();
  else
    src = g_timeout_source_new (timeout);

  g_source_set_priority (src, priority);

  g_source_set_callback (src,
                         callback,
                         user_data,
                         NULL);
  src_id = g_source_attach (src, context);
  g_source_unref (src);

  return src_id;
}

static gboolean
on_depth_frame_main_loop (gpointer user_data)
{
  GFreenectDevice *self = GFREENECT_DEVICE (user_data);

  g_mutex_lock (self->priv->mutex);

  self->priv->depth_frame_src_id = 0;

  if (self->priv->got_depth_frame)
    {
      self->priv->got_depth_frame = FALSE;

      g_signal_emit (self, gfreenect_device_signals[SIGNAL_DEPTH_FRAME], 0, NULL);
    }

  g_mutex_unlock (self->priv->mutex);

  return FALSE;
}


static void
on_depth_frame (freenect_device *dev, void *depth, uint32_t timestamp)
{
  GFreenectDevice *self;

  self = freenect_get_user (dev);

  g_mutex_lock (self->priv->mutex);

  self->priv->got_depth_frame = TRUE;

  memcpy (self->priv->depth_buf, depth, self->priv->depth_mode.bytes);

  if (self->priv->depth_frame_src_id == 0)
    {
      self->priv->depth_frame_src_id = timeout_add (self->priv->glib_context,
                                                    0,
                                                    G_PRIORITY_DEFAULT,
                                                    on_depth_frame_main_loop,
                                                    self);
    }

  g_mutex_unlock (self->priv->mutex);
}

static gboolean
on_video_frame_main_loop (gpointer user_data)
{
  GFreenectDevice *self = GFREENECT_DEVICE (user_data);

  g_mutex_lock (self->priv->mutex);

  self->priv->video_frame_src_id = 0;

  if (self->priv->got_video_frame)
    {
      self->priv->got_video_frame = FALSE;

      g_signal_emit (self,
                     gfreenect_device_signals[SIGNAL_VIDEO_FRAME],
                     0,
                     NULL);
    }

  g_mutex_unlock (self->priv->mutex);

  return FALSE;
}

static void
on_video_frame (freenect_device *dev, void *buf, uint32_t timestamp)
{
  GFreenectDevice *self;

  self = freenect_get_user (dev);

  g_mutex_lock (self->priv->mutex);

  self->priv->got_video_frame = TRUE;

  memcpy (self->priv->video_buf, buf, self->priv->video_mode.bytes);

  if (self->priv->video_frame_src_id == 0)
    {
      self->priv->video_frame_src_id = timeout_add (self->priv->glib_context,
                                                    0,
                                                    G_PRIORITY_DEFAULT,
                                                    on_video_frame_main_loop,
                                                    self);
    }

  g_mutex_unlock (self->priv->mutex);
}

static gboolean
init_sync (GInitable     *initable,
           GCancellable  *cancellable,
           GError       **error)
{
  GFreenectDevice *self = GFREENECT_DEVICE (initable);

  if (freenect_init (&self->priv->ctx, NULL) != 0)
    {
      g_set_error (error,
                   G_IO_ERROR,
                   G_IO_ERROR_NOT_INITIALIZED,
                   "Failed to initialize Kinect sensor context");
      return FALSE;
    }

  freenect_select_subdevices (self->priv->ctx, self->priv->subdevices);

  if (freenect_open_device (self->priv->ctx,
                            &self->priv->dev,
                            self->priv->index) != 0)
    {
      g_set_error (error,
                   G_IO_ERROR,
                   G_IO_ERROR_INVALID_ARGUMENT,
                   "Failed to open Kinect device");
      return FALSE;
    }

  freenect_set_user (self->priv->dev, self);

  freenect_set_depth_callback (self->priv->dev, on_depth_frame);
  freenect_set_video_callback (self->priv->dev, on_video_frame);

  self->priv->user_buf = g_slice_alloc (USER_BUF_SIZE);

  return TRUE;
}

static gpointer
init_in_thread (gpointer _data)
{
  ConstructorData *data = _data;
  GError *error = NULL;

  if (! init_sync (G_INITABLE (data->self), data->cancellable, &error))
    {
      g_simple_async_result_set_from_error (data->result, error);
      g_error_free (error);
    }
  else
    {
      g_object_ref (data->self);
      g_simple_async_result_set_op_res_gpointer (data->result,
                                                 data->self,
                                                 g_object_unref);
    }

  g_simple_async_result_complete_in_idle (data->result);
  g_object_unref (data->self);
  g_object_unref (data->result);

  if (data->cancellable != NULL)
    g_object_unref (data->cancellable);

  g_slice_free (ConstructorData, data);

  return NULL;
}

static void
init_async (GAsyncInitable      *initable,
            int                  io_priority,
            GCancellable        *cancellable,
            GAsyncReadyCallback  callback,
            gpointer             user_data)
{
  ConstructorData *data;

  data = g_slice_new (ConstructorData);

  data->self = initable;
  g_object_ref (initable);

  data->result = g_simple_async_result_new (G_OBJECT (initable),
                                            callback,
                                            user_data,
                                            init_async);

  data->cancellable = cancellable;
  if (data->cancellable != NULL)
    g_object_ref (data->cancellable);

  g_thread_create (init_in_thread, data, TRUE, NULL);
}

static gboolean
init_finish (GAsyncInitable  *initable,
             GAsyncResult    *res,
             GError         **error)
{
  return ! g_simple_async_result_propagate_error (G_SIMPLE_ASYNC_RESULT (res),
                                                  error);
}

static gpointer
dispatch_thread_func (gpointer _data)
{
  GFreenectDevice *self = GFREENECT_DEVICE (_data);
  gboolean abort = FALSE;

  while (! abort)
    {
      if (self->priv->update_tilt_angle)
        {
          g_mutex_lock (self->priv->mutex);

          self->priv->update_tilt_angle = FALSE;

          if (freenect_set_tilt_degs (self->priv->dev, self->priv->tilt_angle) != 0)
            {
              /* @TODO: this error must be reported! */
            }

          g_mutex_unlock (self->priv->mutex);
        }

      if (self->priv->update_led)
        {
          g_mutex_lock (self->priv->mutex);

          self->priv->update_led = FALSE;

          if (freenect_set_led (self->priv->dev, self->priv->led) != 0)
            {
              /* @TODO: this error must be reported! */
            }

          g_mutex_unlock (self->priv->mutex);
        }

      freenect_process_events (self->priv->ctx);

      if (self->priv->abort_dispatch_thread)
        abort = TRUE;
    }

  return NULL;
}

static gboolean
launch_dispatch_thread (GFreenectDevice *self, GError **error)
{
  self->priv->depth_frame_src_id = 0;
  self->priv->video_frame_src_id = 0;

  self->priv->glib_context = g_main_context_get_thread_default ();
  self->priv->abort_dispatch_thread = FALSE;

  self->priv->update_tilt_angle = FALSE;
  self->priv->update_led = FALSE;

  self->priv->dispatch_thread = g_thread_create (dispatch_thread_func,
                                                 self,
                                                 TRUE,
                                                 error);
  return self->priv->dispatch_thread != NULL;
}

/* public methods */

/**
 * gfreenect_device_new:
 * @cancellable: (allow-none):
 * @callback: (scope async):
 * @user_data: (allow-none):
 **/
void
gfreenect_device_new (gint                 device_index,
                      guint                subdevices,
                      GCancellable        *cancellable,
                      GAsyncReadyCallback  callback,
                      gpointer             user_data)
{
  g_async_initable_new_async (GFREENECT_TYPE_DEVICE,
                              G_PRIORITY_DEFAULT,
                              cancellable,
                              callback,
                              user_data,
                              "index", device_index,
                              NULL);
}

/**
 * gfreenect_device_new_finish:
 *
 * Returns: (transfer full):
 **/
GFreenectDevice *
gfreenect_device_new_finish (GAsyncResult *result, GError **error)
{
  GSimpleAsyncResult *res;

  g_return_val_if_fail (G_IS_SIMPLE_ASYNC_RESULT (result), NULL);

  res = G_SIMPLE_ASYNC_RESULT (result);

  if (! g_simple_async_result_propagate_error (res, error))
    {
      GAsyncInitable *initable;

      initable = g_simple_async_result_get_op_res_gpointer (res);

      if (GFREENECT_DEVICE (g_async_initable_new_finish (initable,
                                                         result,
                                                         error)))
        return GFREENECT_DEVICE (initable);
      else
        return NULL;
    }
  else
    {
      return NULL;
    }
}

gboolean
gfreenect_device_start_depth_stream (GFreenectDevice *self, GError **error)
{
  g_return_val_if_fail (GFREENECT_IS_DEVICE (self), FALSE);

  /* free current depth buffer */
  if (self->priv->depth_buf != NULL)
    {
      g_slice_free1 (self->priv->depth_mode.bytes, self->priv->depth_buf);
      self->priv->depth_buf = NULL;
    }

  self->priv->depth_mode = freenect_find_depth_mode (FREENECT_RESOLUTION_MEDIUM,
                                                     self->priv->depth_format);

  if (freenect_set_depth_mode (self->priv->dev, self->priv->depth_mode) != 0)
    {
      g_set_error (error,
                   G_IO_ERROR,
                   G_IO_ERROR_FAILED,
                   "Failed to set depth mode");
      return FALSE;
    }

  if (freenect_start_depth (self->priv->dev) != 0)
    {
      g_set_error (error,
                   G_IO_ERROR,
                   G_IO_ERROR_FAILED,
                   "Failed to start depth stream");
      return FALSE;
    }

  self->priv->depth_buf = g_slice_alloc (self->priv->depth_mode.bytes);
  self->priv->got_depth_frame = FALSE;

  if (self->priv->dispatch_thread == NULL)
    return launch_dispatch_thread (self, error);
  else
    return TRUE;
}

gboolean
gfreenect_device_stop_depth_stream (GFreenectDevice *self)
{
  g_return_if_fail (GFREENECT_IS_DEVICE (self));

  if (freenect_stop_depth (self->priv->dev) != 0)
    {
      g_warning ("Failed to stop depth stream");
      return FALSE;
    }

  return TRUE;
}

gboolean
gfreenect_device_start_video_stream (GFreenectDevice *self, GError **error)
{
  g_return_val_if_fail (GFREENECT_IS_DEVICE (self), FALSE);

  /* free current depth buffer */
  if (self->priv->video_buf != NULL)
    {
      g_slice_free1 (self->priv->video_mode.bytes, self->priv->video_buf);
      self->priv->video_buf = NULL;
    }

  self->priv->video_mode = freenect_find_video_mode (self->priv->video_resolution,
                                                     self->priv->video_format);

  if (freenect_set_video_mode (self->priv->dev, self->priv->video_mode) != 0)
    {
      g_set_error (error,
                   G_IO_ERROR,
                   G_IO_ERROR_FAILED,
                   "Failed to set video mode");
      return FALSE;
    }

  if (freenect_start_video (self->priv->dev) != 0)
    {
      g_set_error (error,
                   G_IO_ERROR,
                   G_IO_ERROR_FAILED,
                   "Failed to start video stream");
      return FALSE;
    }

  self->priv->video_buf = g_slice_alloc (self->priv->video_mode.bytes);
  self->priv->got_video_frame = FALSE;

  if (self->priv->dispatch_thread == NULL)
    return launch_dispatch_thread (self, error);
  else
    return TRUE;
}

void
gfreenect_device_set_led (GFreenectDevice *self, GFreenectLed led)
{
  g_return_if_fail (GFREENECT_IS_DEVICE (self));

  if (self->priv->dispatch_thread == NULL)
    launch_dispatch_thread (self, NULL);

  g_mutex_lock (self->priv->mutex);

  self->priv->led = led;
  self->priv->update_led = TRUE;

  g_mutex_unlock (self->priv->mutex);
}

/**
 * gfreenect_device_get_depth_frame_raw:
 * @len: (out):
 *
 * Returns: (array length=len) (element-type guint8):
 **/
guint8 *
gfreenect_device_get_depth_frame_raw (GFreenectDevice *self, gsize *len)
{
  g_return_val_if_fail (GFREENECT_IS_DEVICE (self), NULL);

  if (len != NULL)
    *len = self->priv->depth_mode.bytes;

  return self->priv->depth_buf;
}

/**
 * gfreenect_device_get_video_frame_raw:
 * @len: (out) (allow-none):
 *
 * Returns: (array length=len) (element-type guint8) (transfer none):
 **/
guint8 *
gfreenect_device_get_video_frame_raw (GFreenectDevice *self, gsize *len)
{
  g_return_val_if_fail (GFREENECT_IS_DEVICE (self), NULL);

  if (len != NULL)
    *len = self->priv->video_mode.bytes;

  return (guint8 *) self->priv->video_buf;
}

/**
 * gfreenect_device_get_depth_frame_grayscale:
 * @len: (out) (allow-none):
 *
 * Returns: (array length=len) (element-type guint8) (transfer none):
 **/
guint8 *
gfreenect_device_get_depth_frame_grayscale (GFreenectDevice *self, gsize *len)
{
  guint8 *rgb_buf;
  gint i;
  gdouble d;
  guchar c;
  guint16 *data;
  gsize pixels;

  g_return_val_if_fail (GFREENECT_IS_DEVICE (self), NULL);

  rgb_buf = (guint8 *) self->priv->user_buf;

  data = self->priv->depth_buf;

  pixels = self->priv->depth_mode.width * self->priv->depth_mode.height;

  for (i=0; i < pixels; i++)
    {
      d = ((double) data[i]) / 2048.0;

      c = round (d * 256);

      rgb_buf[i * 3 + 0] = c;
      rgb_buf[i * 3 + 1] = c;
      rgb_buf[i * 3 + 2] = c;
    }

  if (len != NULL)
    *len = pixels * 3;

  return rgb_buf;
}

void
gfreenect_device_set_tilt_angle (GFreenectDevice *self, gdouble tilt_angle)
{
  g_return_if_fail (GFREENECT_IS_DEVICE (self));

  if (self->priv->dispatch_thread == NULL)
    launch_dispatch_thread (self, NULL);

  g_mutex_lock (self->priv->mutex);

  self->priv->tilt_angle = tilt_angle;
  self->priv->update_tilt_angle = TRUE;

  g_mutex_unlock (self->priv->mutex);
}

gdouble
gfreenect_device_get_tilt_angle (GFreenectDevice *self)
{
  freenect_raw_tilt_state *state;
  g_return_if_fail (GFREENECT_IS_DEVICE (self));

  if (freenect_update_tilt_state (self->priv->dev) == -1)
    {
      g_warning("Failed to get the angle %d", freenect_update_tilt_state (self->priv->dev));
      return 0.0;
    }
  state = freenect_get_tilt_state (self->priv->dev);
  return (gdouble) freenect_get_tilt_degs (state);
}
