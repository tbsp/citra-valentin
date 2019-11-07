// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QSettings>
#include "citra_qt/configuration/config.h"
#include "citra_qt/uisettings.h"

void Config::ReadGameListValues() {
    qt_config->beginGroup(QStringLiteral("GameList"));
    auto icon_size = UISettings::GameListIconSize{
        ReadSetting(QStringLiteral("iconSize"),
                    static_cast<int>(UISettings::GameListIconSize::LargeIcon))
            .toInt()};
    if (icon_size < UISettings::GameListIconSize::NoIcon ||
        icon_size > UISettings::GameListIconSize::LargeIcon) {
        icon_size = UISettings::GameListIconSize::LargeIcon;
    }
    UISettings::values.game_list_icon_size = icon_size;

    UISettings::GameListText row_1 = UISettings::GameListText{
        ReadSetting(QStringLiteral("row1"), static_cast<int>(UISettings::GameListText::TitleName))
            .toInt()};
    if (row_1 <= UISettings::GameListText::NoText || row_1 >= UISettings::GameListText::ListEnd) {
        row_1 = UISettings::GameListText::TitleName;
    }
    UISettings::values.game_list_row_1 = row_1;

    UISettings::GameListText row_2 = UISettings::GameListText{
        ReadSetting(QStringLiteral("row2"), static_cast<int>(UISettings::GameListText::FileName))
            .toInt()};
    if (row_2 < UISettings::GameListText::NoText || row_2 >= UISettings::GameListText::ListEnd) {
        row_2 = UISettings::GameListText::FileName;
    }
    UISettings::values.game_list_row_2 = row_2;

    UISettings::values.game_list_hide_no_icon =
        ReadSetting(QStringLiteral("hideNoIcon"), false).toBool();
    UISettings::values.game_list_single_line_mode =
        ReadSetting(QStringLiteral("singleLineMode"), false).toBool();
    qt_config->endGroup();
}

void Config::SaveGameListValues() {
    qt_config->beginGroup(QStringLiteral("GameList"));
    WriteSetting(QStringLiteral("iconSize"),
                 static_cast<int>(UISettings::values.game_list_icon_size), 2);
    WriteSetting(QStringLiteral("row1"), static_cast<int>(UISettings::values.game_list_row_1), 2);
    WriteSetting(QStringLiteral("row2"), static_cast<int>(UISettings::values.game_list_row_2), 0);
    WriteSetting(QStringLiteral("hideNoIcon"), UISettings::values.game_list_hide_no_icon, false);
    WriteSetting(QStringLiteral("singleLineMode"), UISettings::values.game_list_single_line_mode,
                 false);
    qt_config->endGroup();
}
