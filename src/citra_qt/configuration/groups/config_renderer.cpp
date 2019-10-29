// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QSettings>
#include "citra_qt/configuration/config.h"

void Config::ReadRendererValues() {
    qt_config->beginGroup("Renderer");
    Settings::values.use_hw_renderer = ReadSetting("use_hw_renderer", true).toBool();
    Settings::values.use_hw_shader = ReadSetting("use_hw_shader", true).toBool();
    Settings::values.use_disk_shader_cache = ReadSetting("use_disk_shader_cache", true).toBool();
    Settings::values.shaders_accurate_mul = ReadSetting("shaders_accurate_mul", false).toBool();
    Settings::values.use_shader_jit = ReadSetting("use_shader_jit", true).toBool();
    Settings::values.resolution_factor =
        static_cast<u16>(ReadSetting("resolution_factor", 1).toInt());
    Settings::values.use_frame_limit = ReadSetting("use_frame_limit", true).toBool();
    Settings::values.frame_limit = ReadSetting("frame_limit", 100).toInt();

    Settings::values.bg_red = ReadSetting("bg_red", 0.0).toFloat();
    Settings::values.bg_green = ReadSetting("bg_green", 0.0).toFloat();
    Settings::values.bg_blue = ReadSetting("bg_blue", 0.0).toFloat();
    qt_config->endGroup();
}

void Config::SaveRendererValues() {
    qt_config->beginGroup("Renderer");
    WriteSetting("use_hw_renderer", Settings::values.use_hw_renderer, true);
    WriteSetting("use_hw_shader", Settings::values.use_hw_shader, true);
    WriteSetting("use_disk_shader_cache", Settings::values.use_disk_shader_cache, true);
    WriteSetting("shaders_accurate_mul", Settings::values.shaders_accurate_mul, false);
    WriteSetting("use_shader_jit", Settings::values.use_shader_jit, true);
    WriteSetting("resolution_factor", Settings::values.resolution_factor, 1);
    WriteSetting("use_frame_limit", Settings::values.use_frame_limit, true);
    WriteSetting("frame_limit", Settings::values.frame_limit, 100);

    // Cast to double because Qt's written float values are not human-readable
    WriteSetting("bg_red", (double)Settings::values.bg_red, 0.0);
    WriteSetting("bg_green", (double)Settings::values.bg_green, 0.0);
    WriteSetting("bg_blue", (double)Settings::values.bg_blue, 0.0);
    qt_config->endGroup();
}
