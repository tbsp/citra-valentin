// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QSettings>
#include "citra_qt/configuration/config.h"

void Config::ReadCoreValues() {
    qt_config->beginGroup(QStringLiteral("Core"));
    Settings::values.use_cpu_jit = ReadSetting(QStringLiteral("use_cpu_jit"), true).toBool();
    Settings::values.use_custom_cpu_ticks =
        ReadSetting(QStringLiteral("use_custom_cpu_ticks"), false).toBool();
    Settings::values.custom_cpu_ticks =
        ReadSetting(QStringLiteral("custom_cpu_ticks"), 77).toULongLong();
    Settings::values.cpu_clock_percentage =
        ReadSetting(QStringLiteral("cpu_clock_percentage"), 100).toInt();
    qt_config->endGroup();
}

void Config::SaveCoreValues() {
    qt_config->beginGroup(QStringLiteral("Core"));
    WriteSetting(QStringLiteral("use_cpu_jit"), Settings::values.use_cpu_jit, true);
    WriteSetting(QStringLiteral("use_custom_cpu_ticks"), Settings::values.use_custom_cpu_ticks,
                 false);
    WriteSetting(QStringLiteral("custom_cpu_ticks"),
                 static_cast<qulonglong>(Settings::values.custom_cpu_ticks), 77);
    WriteSetting(QStringLiteral("cpu_clock_percentage"), Settings::values.cpu_clock_percentage,
                 100);
    qt_config->endGroup();
}
