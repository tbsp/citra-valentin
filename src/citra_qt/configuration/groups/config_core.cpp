// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QSettings>
#include "citra_qt/configuration/config.h"

void Config::ReadCoreValues() {
    qt_config->beginGroup(QStringLiteral("Core"));
    Settings::values.use_cpu_jit = ReadSetting(QStringLiteral("use_cpu_jit"), true).toBool();
    Settings::values.cpu_clock_percentage =
        ReadSetting(QStringLiteral("cpu_clock_percentage"), 100).toInt();
    qt_config->endGroup();
}

void Config::SaveCoreValues() {
    qt_config->beginGroup(QStringLiteral("Core"));
    WriteSetting(QStringLiteral("use_cpu_jit"), Settings::values.use_cpu_jit, true);
    WriteSetting(QStringLiteral("cpu_clock_percentage"), Settings::values.cpu_clock_percentage,
                 100);
    qt_config->endGroup();
}
