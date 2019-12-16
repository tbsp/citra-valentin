// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QSettings>
#include "citra_qt/configuration/config.h"

void Config::ReadHacksValues() {
    qt_config->beginGroup(QStringLiteral("Hacks"));
    Settings::values.custom_screen_refresh_rate =
        ReadSetting(QStringLiteral("custom_screen_refresh_rate"), false).toBool();
    Settings::values.screen_refresh_rate =
        ReadSetting(QStringLiteral("screen_refresh_rate"), 60).toDouble();

    Settings::values.sharper_distant_objects =
        ReadSetting(QStringLiteral("sharper_distant_objects"), false).toBool();
    Settings::values.ignore_format_reinterpretation =
        ReadSetting(QStringLiteral("ignore_format_reinterpretation"), true).toBool();
    qt_config->endGroup();
}

void Config::SaveHacksValues() {
    qt_config->beginGroup(QStringLiteral("Hacks"));
    WriteSetting(QStringLiteral("custom_screen_refresh_rate"),
                 Settings::values.custom_screen_refresh_rate, false);
    WriteSetting(QStringLiteral("screen_refresh_rate"), Settings::values.screen_refresh_rate,
                 60.00);

    WriteSetting(QStringLiteral("sharper_distant_objects"),
                 Settings::values.sharper_distant_objects, false);
    WriteSetting(QStringLiteral("ignore_format_reinterpretation"),
                 Settings::values.ignore_format_reinterpretation, true);
    qt_config->endGroup();
}
