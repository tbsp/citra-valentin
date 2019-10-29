// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QSettings>
#include "citra_qt/configuration/config.h"

void Config::ReadUtilityValues() {
    qt_config->beginGroup("Utility");
    Settings::values.dump_textures = ReadSetting("dump_textures", false).toBool();
    Settings::values.custom_textures = ReadSetting("custom_textures", false).toBool();
    Settings::values.preload_textures = ReadSetting("preload_textures", false).toBool();
    qt_config->endGroup();
}

void Config::SaveUtilityValues() {
    qt_config->beginGroup("Utility");
    WriteSetting("dump_textures", Settings::values.dump_textures, false);
    WriteSetting("custom_textures", Settings::values.custom_textures, false);
    WriteSetting("preload_textures", Settings::values.preload_textures, false);
    qt_config->endGroup();
}
