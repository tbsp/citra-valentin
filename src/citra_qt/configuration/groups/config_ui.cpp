// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QSettings>
#include "citra_qt/configuration/config.h"
#include "citra_qt/uisettings.h"

void Config::ReadUiValues() {
    qt_config->beginGroup(QStringLiteral("UI"));
    UISettings::values.theme =
        ReadSetting(QStringLiteral("theme"), UISettings::themes[0].second).toString();
    UISettings::values.screenshot_resolution_factor =
        static_cast<u16>(ReadSetting(QStringLiteral("screenshot_resolution_factor"), 0).toUInt());

    UISettings::values.single_window_mode =
        ReadSetting(QStringLiteral("singleWindowMode"), true).toBool();
    UISettings::values.fullscreen = ReadSetting(QStringLiteral("fullscreen"), false).toBool();
    UISettings::values.display_titlebar =
        ReadSetting(QStringLiteral("displayTitleBars"), true).toBool();
    UISettings::values.show_filter_bar =
        ReadSetting(QStringLiteral("showFilterBar"), true).toBool();
    UISettings::values.show_status_bar =
        ReadSetting(QStringLiteral("showStatusBar"), true).toBool();
    UISettings::values.confirm_before_closing =
        ReadSetting(QStringLiteral("confirmClose"), true).toBool();
    UISettings::values.show_console = ReadSetting(QStringLiteral("showConsole"), false).toBool();
    UISettings::values.pause_when_in_background =
        ReadSetting(QStringLiteral("pauseWhenInBackground"), false).toBool();

    ReadUiLayoutValues();
    ReadGameListValues();
    ReadPathsValues();
    ReadShortcutsValues();
    ReadMultiplayerValues();
    qt_config->endGroup();
}

void Config::SaveUiValues() {
    qt_config->beginGroup(QStringLiteral("UI"));
    WriteSetting(QStringLiteral("theme"), UISettings::values.theme, UISettings::themes[0].second);
    WriteSetting(QStringLiteral("screenshot_resolution_factor"),
                 UISettings::values.screenshot_resolution_factor, 0);
    WriteSetting(QStringLiteral("singleWindowMode"), UISettings::values.single_window_mode, true);
    WriteSetting(QStringLiteral("fullscreen"), UISettings::values.fullscreen, false);
    WriteSetting(QStringLiteral("displayTitleBars"), UISettings::values.display_titlebar, true);
    WriteSetting(QStringLiteral("showFilterBar"), UISettings::values.show_filter_bar, true);
    WriteSetting(QStringLiteral("showStatusBar"), UISettings::values.show_status_bar, true);
    WriteSetting(QStringLiteral("confirmClose"), UISettings::values.confirm_before_closing, true);
    WriteSetting(QStringLiteral("showConsole"), UISettings::values.show_console, false);
    WriteSetting(QStringLiteral("pauseWhenInBackground"),
                 UISettings::values.pause_when_in_background, false);

    SaveUiLayoutValues();
    SaveGameListValues();
    SavePathsValues();
    SaveShortcutsValues();
    SaveMultiplayerValues();
    qt_config->endGroup();
}
