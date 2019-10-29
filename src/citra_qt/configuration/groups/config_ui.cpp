// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QSettings>
#include "citra_qt/configuration/config.h"
#include "citra_qt/uisettings.h"

void Config::ReadUiValues() {
    qt_config->beginGroup("UI");
    UISettings::values.theme = ReadSetting("theme", UISettings::themes[0].second).toString();
    UISettings::values.screenshot_resolution_factor =
        static_cast<u16>(ReadSetting("screenshot_resolution_factor", 0).toUInt());

    UISettings::values.single_window_mode = ReadSetting("singleWindowMode", true).toBool();
    UISettings::values.fullscreen = ReadSetting("fullscreen", false).toBool();
    UISettings::values.display_titlebar = ReadSetting("displayTitleBars", true).toBool();
    UISettings::values.show_filter_bar = ReadSetting("showFilterBar", true).toBool();
    UISettings::values.show_status_bar = ReadSetting("showStatusBar", true).toBool();
    UISettings::values.confirm_before_closing = ReadSetting("confirmClose", true).toBool();
    UISettings::values.show_console = ReadSetting("showConsole", false).toBool();
    UISettings::values.pause_when_in_background =
        ReadSetting("pauseWhenInBackground", false).toBool();

    ReadUiLayoutValues();
    ReadGameListValues();
    ReadPathsValues();
    ReadShortcutsValues();
    ReadMultiplayerValues();
    qt_config->endGroup();
}

void Config::SaveUiValues() {
    qt_config->beginGroup("UI");
    WriteSetting("theme", UISettings::values.theme, UISettings::themes[0].second);
    WriteSetting("screenshot_resolution_factor", UISettings::values.screenshot_resolution_factor,
                 0);
    WriteSetting("singleWindowMode", UISettings::values.single_window_mode, true);
    WriteSetting("fullscreen", UISettings::values.fullscreen, false);
    WriteSetting("displayTitleBars", UISettings::values.display_titlebar, true);
    WriteSetting("showFilterBar", UISettings::values.show_filter_bar, true);
    WriteSetting("showStatusBar", UISettings::values.show_status_bar, true);
    WriteSetting("confirmClose", UISettings::values.confirm_before_closing, true);
    WriteSetting("showConsole", UISettings::values.show_console, false);
    WriteSetting("pauseWhenInBackground", UISettings::values.pause_when_in_background, false);

    SaveUiLayoutValues();
    SaveGameListValues();
    SavePathsValues();
    SaveShortcutsValues();
    SaveMultiplayerValues();
    qt_config->endGroup();
}
