// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QSettings>
#include "citra_qt/configuration/config.h"

void Config::ReadSystemValues() {
    qt_config->beginGroup(QStringLiteral("System"));
    Settings::values.is_new_3ds = ReadSetting(QStringLiteral("is_new_3ds"), false).toBool();
    Settings::values.region_value =
        ReadSetting(QStringLiteral("region_value"), Settings::REGION_VALUE_AUTO_SELECT).toInt();
    Settings::values.init_clock = static_cast<Settings::InitClock>(
        ReadSetting(QStringLiteral("init_clock"), static_cast<u32>(Settings::InitClock::SystemTime))
            .toInt());
    Settings::values.init_time =
        ReadSetting(QStringLiteral("init_time"), 946681277ULL).toULongLong();
    qt_config->endGroup();
}

void Config::SaveSystemValues() {
    qt_config->beginGroup(QStringLiteral("System"));
    WriteSetting(QStringLiteral("is_new_3ds"), Settings::values.is_new_3ds, false);
    WriteSetting(QStringLiteral("region_value"), Settings::values.region_value,
                 Settings::REGION_VALUE_AUTO_SELECT);
    WriteSetting(QStringLiteral("init_clock"), static_cast<u32>(Settings::values.init_clock),
                 static_cast<u32>(Settings::InitClock::SystemTime));
    WriteSetting(QStringLiteral("init_time"),
                 static_cast<unsigned long long>(Settings::values.init_time), 946681277ULL);
    qt_config->endGroup();
}
