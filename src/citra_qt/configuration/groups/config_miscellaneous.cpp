// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QSettings>
#include "citra_qt/configuration/config.h"

#ifdef CITRA_ENABLE_DISCORD_RP
#include "citra_qt/uisettings.h"
#endif

void Config::ReadMiscellaneousValues() {
    qt_config->beginGroup(QStringLiteral("Miscellaneous"));
    Settings::values.log_filter =
        ReadSetting(QStringLiteral("log_filter"), QStringLiteral("*:Info"))
            .toString()
            .toStdString();
#ifdef CITRA_ENABLE_DISCORD_RP
    UISettings::values.enable_discord_rp =
        ReadSetting(QStringLiteral("enable_discord_rp"), true).toBool();
    UISettings::values.discord_rp_show_game_name =
        ReadSetting(QStringLiteral("discord_rp_show_game_name"), true).toBool();
    UISettings::values.discord_rp_show_room_information =
        ReadSetting(QStringLiteral("discord_rp_show_room_information"), true).toBool();
#endif
    qt_config->endGroup();
}

void Config::SaveMiscellaneousValues() {
    qt_config->beginGroup(QStringLiteral("Miscellaneous"));
    WriteSetting(QStringLiteral("log_filter"), QString::fromStdString(Settings::values.log_filter),
                 QStringLiteral("*:Info"));
#ifdef CITRA_ENABLE_DISCORD_RP
    WriteSetting(QStringLiteral("enable_discord_rp"), UISettings::values.enable_discord_rp, true);
    WriteSetting(QStringLiteral("discord_rp_show_game_name"),
                 UISettings::values.discord_rp_show_game_name, true);
    WriteSetting(QStringLiteral("discord_rp_show_room_information"),
                 UISettings::values.discord_rp_show_room_information, true);
#endif
    qt_config->endGroup();
}
