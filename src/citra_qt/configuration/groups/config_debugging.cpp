// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QSettings>
#include "citra_qt/configuration/config.h"

void Config::ReadDebuggingValues() {
    qt_config->beginGroup("Debugging");
    // Intentionally not using the QT default setting as this is intended to be changed in the ini
    Settings::values.record_frame_times = qt_config->value("record_frame_times", false).toBool();
    Settings::values.use_gdbstub = ReadSetting("use_gdbstub", false).toBool();
    Settings::values.gdbstub_port = ReadSetting("gdbstub_port", 24689).toInt();

    ReadLleValues();
    qt_config->endGroup();
}

void Config::SaveDebuggingValues() {
    qt_config->beginGroup("Debugging");
    // Intentionally not using the QT default setting as this is intended to be changed in the ini
    qt_config->setValue("record_frame_times", Settings::values.record_frame_times);
    WriteSetting("use_gdbstub", Settings::values.use_gdbstub, false);
    WriteSetting("gdbstub_port", Settings::values.gdbstub_port, 24689);

    SaveLleValues();
    qt_config->endGroup();
}
