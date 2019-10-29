// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QSettings>
#include "citra_qt/configuration/config.h"
#include "citra_qt/uisettings.h"

void Config::ReadUiLayoutValues() {
    qt_config->beginGroup("UILayout");
    UISettings::values.geometry = ReadSetting("geometry").toByteArray();
    UISettings::values.state = ReadSetting("state").toByteArray();
    UISettings::values.renderwindow_geometry = ReadSetting("geometryRenderWindow").toByteArray();
    UISettings::values.gamelist_header_state = ReadSetting("gameListHeaderState").toByteArray();
    UISettings::values.microprofile_geometry =
        ReadSetting("microProfileDialogGeometry").toByteArray();
    UISettings::values.microprofile_visible =
        ReadSetting("microProfileDialogVisible", false).toBool();
    qt_config->endGroup();
}

void Config::SaveUiLayoutValues() {
    qt_config->beginGroup("UILayout");
    WriteSetting("geometry", UISettings::values.geometry);
    WriteSetting("state", UISettings::values.state);
    WriteSetting("geometryRenderWindow", UISettings::values.renderwindow_geometry);
    WriteSetting("gameListHeaderState", UISettings::values.gamelist_header_state);
    WriteSetting("microProfileDialogGeometry", UISettings::values.microprofile_geometry);
    WriteSetting("microProfileDialogVisible", UISettings::values.microprofile_visible, false);
    qt_config->endGroup();
}
