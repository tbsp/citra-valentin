// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QSettings>
#include "citra_qt/configuration/config.h"

void Config::ReadHacksValues() {
    qt_config->beginGroup("Hacks");
    Settings::values.custom_ticks = ReadSetting("custom_ticks", false).toBool();
    Settings::values.ticks = ReadSetting("ticks", 77).toULongLong();

    Settings::values.custom_screen_refresh_rate =
        ReadSetting("custom_screen_refresh_rate", false).toBool();
    Settings::values.screen_refresh_rate = ReadSetting("screen_refresh_rate", 60).toDouble();

    Settings::values.sharper_distant_objects =
        ReadSetting("sharper_distant_objects", false).toBool();
    Settings::values.ignore_format_reinterpretation =
        ReadSetting("ignore_format_reinterpretation", true).toBool();
    qt_config->endGroup();
}

void Config::SaveHacksValues() {
    qt_config->beginGroup("Hacks");
    WriteSetting("custom_ticks", Settings::values.custom_ticks, false);
    WriteSetting("ticks", static_cast<qulonglong>(Settings::values.ticks), 77);

    WriteSetting("custom_screen_refresh_rate", Settings::values.custom_screen_refresh_rate, false);
    WriteSetting("screen_refresh_rate", Settings::values.screen_refresh_rate, 60.00);

    WriteSetting("sharper_distant_objects", Settings::values.sharper_distant_objects, false);
    WriteSetting("ignore_format_reinterpretation", Settings::values.ignore_format_reinterpretation,
                 true);
    qt_config->endGroup();
}
