// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QSettings>
#include "citra_qt/configuration/config.h"

void Config::ReadLayoutValues() {
    qt_config->beginGroup(QStringLiteral("Layout"));
    Settings::values.render_3d = static_cast<Settings::StereoRenderOption>(
        ReadSetting(QStringLiteral("render_3d"), 0).toInt());
    Settings::values.factor_3d = ReadSetting(QStringLiteral("factor_3d"), 0).toInt();
    Settings::values.pp_shader_name =
        ReadSetting(QStringLiteral("pp_shader_name"),
                    (Settings::values.render_3d == Settings::StereoRenderOption::Anaglyph)
                        ? QStringLiteral("dubois (builtin)")
                        : QStringLiteral("none (builtin)"))
            .toString()
            .toStdString();
    Settings::values.filter_mode = ReadSetting(QStringLiteral("filter_mode"), true).toBool();
    Settings::values.layout_option =
        static_cast<Settings::LayoutOption>(ReadSetting(QStringLiteral("layout_option")).toInt());
    Settings::values.swap_screen = ReadSetting(QStringLiteral("swap_screen"), false).toBool();
    Settings::values.custom_layout = ReadSetting(QStringLiteral("custom_layout"), false).toBool();
    Settings::values.custom_top_left = ReadSetting(QStringLiteral("custom_top_left"), 0).toInt();
    Settings::values.custom_top_top = ReadSetting(QStringLiteral("custom_top_top"), 0).toInt();
    Settings::values.custom_top_right =
        ReadSetting(QStringLiteral("custom_top_right"), 400).toInt();
    Settings::values.custom_top_bottom =
        ReadSetting(QStringLiteral("custom_top_bottom"), 240).toInt();
    Settings::values.custom_bottom_left =
        ReadSetting(QStringLiteral("custom_bottom_left"), 40).toInt();
    Settings::values.custom_bottom_top =
        ReadSetting(QStringLiteral("custom_bottom_top"), 240).toInt();
    Settings::values.custom_bottom_right =
        ReadSetting(QStringLiteral("custom_bottom_right"), 360).toInt();
    Settings::values.custom_bottom_bottom =
        ReadSetting(QStringLiteral("custom_bottom_bottom"), 480).toInt();
    qt_config->endGroup();
}

void Config::SaveLayoutValues() {
    qt_config->beginGroup(QStringLiteral("Layout"));
    WriteSetting(QStringLiteral("render_3d"), static_cast<int>(Settings::values.render_3d), 0);
    WriteSetting(QStringLiteral("factor_3d"), Settings::values.factor_3d.load(), 0);
    WriteSetting(QStringLiteral("pp_shader_name"),
                 QString::fromStdString(Settings::values.pp_shader_name),
                 (Settings::values.render_3d == Settings::StereoRenderOption::Anaglyph)
                     ? QStringLiteral("dubois (builtin)")
                     : QStringLiteral("none (builtin)"));
    WriteSetting(QStringLiteral("filter_mode"), Settings::values.filter_mode, true);
    WriteSetting(QStringLiteral("layout_option"), static_cast<int>(Settings::values.layout_option));
    WriteSetting(QStringLiteral("swap_screen"), Settings::values.swap_screen, false);
    WriteSetting(QStringLiteral("custom_layout"), Settings::values.custom_layout, false);
    WriteSetting(QStringLiteral("custom_top_left"), Settings::values.custom_top_left, 0);
    WriteSetting(QStringLiteral("custom_top_top"), Settings::values.custom_top_top, 0);
    WriteSetting(QStringLiteral("custom_top_right"), Settings::values.custom_top_right, 400);
    WriteSetting(QStringLiteral("custom_top_bottom"), Settings::values.custom_top_bottom, 240);
    WriteSetting(QStringLiteral("custom_bottom_left"), Settings::values.custom_bottom_left, 40);
    WriteSetting(QStringLiteral("custom_bottom_top"), Settings::values.custom_bottom_top, 240);
    WriteSetting(QStringLiteral("custom_bottom_right"), Settings::values.custom_bottom_right, 360);
    WriteSetting(QStringLiteral("custom_bottom_bottom"), Settings::values.custom_bottom_bottom,
                 480);
    qt_config->endGroup();
}
