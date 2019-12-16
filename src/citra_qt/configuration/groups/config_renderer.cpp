// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QSettings>
#include "citra_qt/configuration/config.h"

void Config::ReadRendererValues() {
    qt_config->beginGroup(QStringLiteral("Renderer"));
    Settings::values.use_hw_renderer =
        ReadSetting(QStringLiteral("use_hw_renderer"), true).toBool();
    Settings::values.use_hw_shader = ReadSetting(QStringLiteral("use_hw_shader"), true).toBool();
    Settings::values.enable_disk_shader_cache =
        ReadSetting(QStringLiteral("enable_disk_shader_cache"), false).toBool();
    Settings::values.shaders_accurate_mul =
        ReadSetting(QStringLiteral("shaders_accurate_mul"), false).toBool();
    Settings::values.use_shader_jit = ReadSetting(QStringLiteral("use_shader_jit"), true).toBool();
    Settings::values.resolution_factor =
        static_cast<u16>(ReadSetting(QStringLiteral("resolution_factor"), 1).toInt());
    Settings::values.use_frame_limit =
        ReadSetting(QStringLiteral("use_frame_limit"), true).toBool();
    Settings::values.frame_limit = ReadSetting(QStringLiteral("frame_limit"), 100).toInt();

    Settings::values.bg_red = ReadSetting(QStringLiteral("bg_red"), 0.0).toFloat();
    Settings::values.bg_green = ReadSetting(QStringLiteral("bg_green"), 0.0).toFloat();
    Settings::values.bg_blue = ReadSetting(QStringLiteral("bg_blue"), 0.0).toFloat();
    Settings::values.min_vertices_per_thread =
        ReadSetting(QStringLiteral("min_vertices_per_thread"), 10).toInt();
    Settings::values.use_vsync_new = ReadSetting(QStringLiteral("use_vsync_new"), true).toBool();
    qt_config->endGroup();
}

void Config::SaveRendererValues() {
    qt_config->beginGroup(QStringLiteral("Renderer"));
    WriteSetting(QStringLiteral("use_hw_renderer"), Settings::values.use_hw_renderer, true);
    WriteSetting(QStringLiteral("use_hw_shader"), Settings::values.use_hw_shader, true);
    WriteSetting(QStringLiteral("enable_disk_shader_cache"),
                 Settings::values.enable_disk_shader_cache, false);
    WriteSetting(QStringLiteral("shaders_accurate_mul"), Settings::values.shaders_accurate_mul,
                 false);
    WriteSetting(QStringLiteral("use_shader_jit"), Settings::values.use_shader_jit, true);
    WriteSetting(QStringLiteral("resolution_factor"), Settings::values.resolution_factor, 1);
    WriteSetting(QStringLiteral("use_frame_limit"), Settings::values.use_frame_limit, true);
    WriteSetting(QStringLiteral("frame_limit"), Settings::values.frame_limit, 100);
    WriteSetting(QStringLiteral("min_vertices_per_thread"),
                 Settings::values.min_vertices_per_thread, 10);
    WriteSetting(QStringLiteral("use_vsync_new"), Settings::values.use_vsync_new, true);

    // Cast to double because Qt's written float values are not human-readable
    WriteSetting(QStringLiteral("bg_red"), (double)Settings::values.bg_red, 0.0);
    WriteSetting(QStringLiteral("bg_green"), (double)Settings::values.bg_green, 0.0);
    WriteSetting(QStringLiteral("bg_blue"), (double)Settings::values.bg_blue, 0.0);
    qt_config->endGroup();
}
