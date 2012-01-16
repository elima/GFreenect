# -*- coding: utf-8 -*-
#
# testview.py
#
# gfreenect - A GObject wrapper of the libfreenect library
# Copyright (C) 2011 Igalia S.L.
#
# Authors:
#   Joaquim Manuel Pereira Rocha <jrocha@igalia.com>
#   Eduardo Lima Mitev <elima@igalia.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 3 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# Lesser General Public License at http://www.gnu.org/licenses/lgpl-3.0.txt
# for more details.
#

import sys
from gi.repository import GFreenect
from gi.repository import Clutter
from gi.repository import Gdk
from gi.repository import Gtk, GObject
from gi.repository import GtkClutter

class GFreenectView(Gtk.Window):

    def __init__(self):
        Gtk.Window.__init__(self, type=Gtk.WindowType.TOPLEVEL)
        self.set_title('GFreenect View')
        self.connect('delete-event', self._on_delete_event)
        self.set_size_request(800, 600)

        contents = Gtk.Box.new(Gtk.Orientation.VERTICAL, 12)
        self.add(contents)

        top_contents = Gtk.Box.new(Gtk.Orientation.HORIZONTAL, 12)
        contents.pack_start(top_contents, fill=True, expand=True, padding=0)
        embed = GtkClutter.Embed.new()
        top_contents.pack_start(embed, fill=True, expand=True, padding=12)

        stage = embed.get_stage()
        stage.set_title('GFreenect View')
        stage.set_user_resizable(True)
        stage.set_color(Clutter.Color.new(0, 0, 0, 255))

        layout_manager = Clutter.BoxLayout()
        textures_box = Clutter.Box.new(layout_manager)
        stage.add_actor(textures_box)
        geometry = stage.get_geometry()
        textures_box.set_geometry(geometry)
        stage.connect('allocation-changed',
                      self._on_allocation_changed,
                      textures_box)

        self._tilt_scale_timeout = 0
        self._tilt_scale = self._create_tilt_scale()
        top_right_contents = Gtk.Box.new(Gtk.Orientation.VERTICAL, 12)
        top_contents.pack_start(top_right_contents, fill=False,
                                expand=False, padding=12)
        label = Gtk.Label()
        label.set_label('Tilt _Motor:')
        label.set_use_underline(True)
        label.set_mnemonic_widget(self._tilt_scale)

        top_right_contents.pack_start(label, fill=False,
                                expand=False, padding=0)
        top_right_contents.pack_start(self._tilt_scale, fill=True,
                                expand=True, padding=0)

        middle_contents = Gtk.Box(Gtk.Orientation.HORIZONTAL, 12)
        contents.pack_start(middle_contents, fill=False, expand=False, padding=0)

        self.rgb_format_radio = Gtk.RadioButton.new_with_label(None, 'RGB Format')
        self.rgb_format_radio.connect('toggled', self._on_video_format_radio_clicked)
        self.ir_format_radio = Gtk.RadioButton.new_from_widget(self.rgb_format_radio)
        self.ir_format_radio.set_label('IR Format')
        middle_contents.pack_start(self.rgb_format_radio, fill=False, expand=False, padding=12)
        middle_contents.pack_start(self.ir_format_radio, fill=False, expand=False, padding=0)

        self.led_combobox = self._create_led_combobox()
        label = Gtk.Label()
        label.set_text('_LED:')
        label.set_use_underline(True)
        label.set_mnemonic_widget(self.led_combobox)
        bottom_contents = Gtk.Box(Gtk.Orientation.HORIZONTAL, 12)
        contents.pack_end(bottom_contents, fill=False, expand=False, padding=0)
        bottom_contents.pack_start(label, fill=False, expand=False, padding=12)
        bottom_contents.pack_start(self.led_combobox, fill=False, expand=False, padding=0)

        self._accel_timeout = 0
        self._accel_x_label = Gtk.Label()
        self._accel_y_label = Gtk.Label()
        self._accel_z_label = Gtk.Label()
        label = Gtk.Label()
        label.set_text('Accelerometer:')
        bottom_contents.pack_start(label, fill=False, expand=False, padding=0)
        bottom_contents.pack_start(self._accel_x_label, fill=False, expand=False, padding=6)
        bottom_contents.pack_start(self._accel_y_label, fill=False, expand=False, padding=6)
        bottom_contents.pack_start(self._accel_z_label, fill=False, expand=False, padding=6)

        self.kinect = None
        GFreenect.Device.new(0,
                             GFreenect.Subdevice.ALL,
                             None,
                             self._on_kinect_ready,
                             layout_manager)

        self.show_all()

    def _create_tilt_scale(self):
        tilt_scale = Gtk.Scale.new_with_range(Gtk.Orientation.VERTICAL,
                                              -31, 31, 1)
        tilt_scale.set_value(0)
        tilt_scale.add_mark(31, Gtk.PositionType.LEFT, '-31 º')
        tilt_scale.add_mark(0, Gtk.PositionType.LEFT, '0 º')
        tilt_scale.add_mark(-31, Gtk.PositionType.LEFT, '31 º')
        tilt_scale.connect('value-changed', self._on_scale_value_changed)
        tilt_scale.connect('format-value', self._on_scale_format_value)
        return tilt_scale

    def _create_led_combobox(self):
        model = Gtk.TreeStore(str, int)
        model.append(None, ['Off', GFreenect.Led.OFF])
        model.append(None, ['Green', GFreenect.Led.GREEN])
        model.append(None, ['Red', GFreenect.Led.RED])
        model.append(None, ['Blink Green', GFreenect.Led.BLINK_GREEN])
        model.append(None, ['Blink Red & Yellow', GFreenect.Led.BLINK_RED_YELLOW])
        led_combobox = Gtk.ComboBoxText.new()
        led_combobox.set_model(model)
        led_combobox.set_active(1)
        led_combobox.connect('changed', self._on_combobox_changed)
        return led_combobox

    def _on_set_tilt_finish(self, kinect, result, user_data):
        try:
            kinect.set_tilt_angle_finish(result)
        except:
            pass
        self._tilt_scale.set_sensitive(True)

    def _on_kinect_ready(self, kinect, result, layout_manager):
        self.kinect = kinect
        try:
            self.kinect.new_finish(result)
        except Exception, e:
            dialog = self._create_error_dialog('Error:\n %s' % e.message)
            dialog.run()
            dialog.destroy()
            Gtk.main_quit()
            return
        self.kinect.set_led(GFreenect.Led.GREEN)
        self.kinect.set_tilt_angle(self._tilt_scale.get_value(),
                                   None,
                                   self._on_set_tilt_finish,
                                   None);
        self.kinect.connect("depth-frame",
                            self._on_depth_frame,
                            None)
        self.kinect.connect("video-frame",
                            self._on_video_frame,
                            None)

        try:
            self.kinect.start_depth_stream(getattr(GFreenect.DepthFormat, '11BIT'))
        except Exception, e:
            print e.message
            Gtk.main_quit()

        try:
            self.kinect.start_video_stream(GFreenect.Resolution.MEDIUM,
                                           GFreenect.VideoFormat.RGB)
        except Exception, e:
            print e.message
            Gtk.main_quit()

        self._get_accel()

        self.depth_texture = Clutter.Texture.new()
        self.depth_texture.set_keep_aspect_ratio(True)
        self.video_texture = Clutter.Texture.new()
        self.video_texture.set_keep_aspect_ratio(True)
        layout_manager.pack(self.depth_texture, expand=False,
                            x_fill=False, y_fill=False,
                            x_align=Clutter.BoxAlignment.CENTER,
                            y_align=Clutter.BoxAlignment.CENTER)
        layout_manager.pack(self.video_texture, expand=False,
                            x_fill=False, y_fill=False,
                            x_align=Clutter.BoxAlignment.CENTER,
                            y_align=Clutter.BoxAlignment.CENTER)

    def _on_depth_frame(self, kinect, user_data):
        data, frame_mode = kinect.get_depth_frame_grayscale()
        self.depth_texture.set_from_rgb_data(data, False,
                                             frame_mode.width, frame_mode.height,
                                             0, frame_mode.bits_per_pixel / 8, 0)

    def _on_video_frame(self, kinect, user_data):
        data, frame_mode = kinect.get_video_frame_rgb()
        self.video_texture.set_from_rgb_data(data, False,
                                             frame_mode.width, frame_mode.height,
                                             0, frame_mode.bits_per_pixel / 8, 0)

    def _on_allocation_changed(self, actor, box, flags, textures_box):
        textures_box.set_geometry(actor.get_geometry())

    def _on_scale_value_changed(self, scale):
        if self._tilt_scale_timeout > 0:
            GObject.source_remove(self._tilt_scale_timeout)
        self._tilt_scale_timeout = GObject.timeout_add(500, self._scale_value_changed_timeout)

    def _on_scale_format_value(self, scale, value):
        return '%s º   ' % int(-value)

    def _scale_value_changed_timeout(self):
        self._tilt_scale.set_sensitive(False)
        self.kinect.set_tilt_angle(-self._tilt_scale.get_value(),
                                   None,
                                   self._on_set_tilt_finish,
                                   None)

    def _on_combobox_changed(self, combobox):
        model = combobox.get_model()
        iter = combobox.get_active_iter()
        led_mode = model.get_value(iter, 1)
        self.kinect.set_led(led_mode)

    def _create_error_dialog(self, message):
        return Gtk.MessageDialog(self,
                                 Gtk.DialogFlags.MODAL | Gtk.DialogFlags.DESTROY_WITH_PARENT,
                                 Gtk.MessageType.ERROR,
                                 Gtk.ButtonsType.CLOSE,
                                 message)

    def _get_accel(self):
        self.kinect.get_accel(None, self._on_accel_finish, None)

    def _on_accel_finish(self, kinect, result, user_data):
        success, x, y, z = kinect.get_accel_finish(result)
        if success:
            self._accel_x_label.set_markup('<b>X:</b> %s' % x)
            self._accel_y_label.set_markup('<b>Y:</b> %s' % y)
            self._accel_z_label.set_markup('<b>Z:</b> %s' % z)
        if self._accel_timeout > 0:
            GObject.source_remove(self._accel_timeout)
        self._accel_timeout = GObject.timeout_add(250, self._get_accel)

    def _on_video_format_radio_clicked(self, rgb_radio):
        self.kinect.stop_video_stream()
        if rgb_radio.get_active():
            video_format = GFreenect.VideoFormat.RGB
        else:
            video_format = GFreenect.VideoFormat.IR_8BIT
        self.kinect.start_video_stream(GFreenect.Resolution.MEDIUM,
                                       video_format)

    def _on_delete_event(self, window, event):
        if self.kinect:
            self.kinect.stop_depth_stream()
        Gtk.main_quit()

if __name__ == '__main__':
    Clutter.init(sys.argv)
    view = GFreenectView()
    Gtk.main()
