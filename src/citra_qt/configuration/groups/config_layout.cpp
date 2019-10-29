// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QSettings>
#include "citra_qt/configuration/config.h"

void Config::ReadLayoutValues() {
    qt_config->beginGroup("Layout");
    Settings::values.render_3d =
        static_cast<Settings::StereoRenderOption>(ReadSetting("render_3d", 0).toInt());
    Settings::values.factor_3d = ReadSetting("factor_3d", 0).toInt();
    Settings::values.pp_shader_name =
        ReadSetting("pp_shader_name",
                    (Settings::values.render_3d == Settings::StereoRenderOption::Anaglyph)
                        ? "dubois (builtin)"
                        : "none (builtin)")
            .toString()
            .toStdString();
    Settings::values.filter_mode = ReadSetting("filter_mode", true).toBool();
    Settings::values.layout_option =
        static_cast<Settings::LayoutOption>(ReadSetting("layout_option").toInt());
    Settings::values.swap_screen = ReadSetting("swap_screen", false).toBool();
    Settings::values.custom_layout = ReadSetting("custom_layout", false).toBool();
    Settings::values.custom_top_left = ReadSetting("custom_top_left", 0).toInt();
    Settings::values.custom_top_top = ReadSetting("custom_top_top", 0).toInt();
    Settings::values.custom_top_right = ReadSetting("custom_top_right", 400).toInt();
    Settings::values.custom_top_bottom = ReadSetting("custom_top_bottom", 240).toInt();
    Settings::values.custom_bottom_left = ReadSetting("custom_bottom_left", 40).toInt();
    Settings::values.custom_bottom_top = ReadSetting("custom_bottom_top", 240).toInt();
    Settings::values.custom_bottom_right = ReadSetting("custom_bottom_right", 360).toInt();
    Settings::values.custom_bottom_bottom = ReadSetting("custom_bottom_bottom", 480).toInt();
    qt_config->endGroup();
}

void Config::SaveLayoutValues() {
    qt_config->beginGroup("Layout");
    WriteSetting("render_3d", static_cast<int>(Settings::values.render_3d), 0);
    WriteSetting("factor_3d", Settings::values.factor_3d.load(), 0);
    WriteSetting("pp_shader_name", QString::fromStdString(Settings::values.pp_shader_name),
                 (Settings::values.render_3d == Settings::StereoRenderOption::Anaglyph)
                     ? "dubois (builtin)"
                     : "none (builtin)");
    WriteSetting("filter_mode", Settings::values.filter_mode, true);
    WriteSetting("layout_option", static_cast<int>(Settings::values.layout_option));
    WriteSetting("swap_screen", Settings::values.swap_screen, false);
    WriteSetting("custom_layout", Settings::values.custom_layout, false);
    WriteSetting("custom_top_left", Settings::values.custom_top_left, 0);
    WriteSetting("custom_top_top", Settings::values.custom_top_top, 0);
    WriteSetting("custom_top_right", Settings::values.custom_top_right, 400);
    WriteSetting("custom_top_bottom", Settings::values.custom_top_bottom, 240);
    WriteSetting("custom_bottom_left", Settings::values.custom_bottom_left, 40);
    WriteSetting("custom_bottom_top", Settings::values.custom_bottom_top, 240);
    WriteSetting("custom_bottom_right", Settings::values.custom_bottom_right, 360);
    WriteSetting("custom_bottom_bottom", Settings::values.custom_bottom_bottom, 480);
    qt_config->endGroup();
}
