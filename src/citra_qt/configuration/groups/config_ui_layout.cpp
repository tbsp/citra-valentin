// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QSettings>
#include "citra_qt/configuration/config.h"
#include "citra_qt/uisettings.h"

void Config::ReadUiLayoutValues() {
    qt_config->beginGroup(QStringLiteral("UILayout"));
    UISettings::values.geometry = ReadSetting(QStringLiteral("geometry")).toByteArray();
    UISettings::values.state = ReadSetting(QStringLiteral("state")).toByteArray();
    UISettings::values.renderwindow_geometry =
        ReadSetting(QStringLiteral("geometryRenderWindow")).toByteArray();
    UISettings::values.gamelist_header_state =
        ReadSetting(QStringLiteral("gameListHeaderState")).toByteArray();
    UISettings::values.microprofile_geometry =
        ReadSetting(QStringLiteral("microProfileDialogGeometry")).toByteArray();
    UISettings::values.microprofile_visible =
        ReadSetting(QStringLiteral("microProfileDialogVisible"), false).toBool();
    qt_config->endGroup();
}

void Config::SaveUiLayoutValues() {
    qt_config->beginGroup(QStringLiteral("UILayout"));
    WriteSetting(QStringLiteral("geometry"), UISettings::values.geometry);
    WriteSetting(QStringLiteral("state"), UISettings::values.state);
    WriteSetting(QStringLiteral("geometryRenderWindow"), UISettings::values.renderwindow_geometry);
    WriteSetting(QStringLiteral("gameListHeaderState"), UISettings::values.gamelist_header_state);
    WriteSetting(QStringLiteral("microProfileDialogGeometry"),
                 UISettings::values.microprofile_geometry);
    WriteSetting(QStringLiteral("microProfileDialogVisible"),
                 UISettings::values.microprofile_visible, false);
    qt_config->endGroup();
}
