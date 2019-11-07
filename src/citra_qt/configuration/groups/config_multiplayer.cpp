// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QSettings>
#include "citra_qt/configuration/config.h"
#include "citra_qt/uisettings.h"
#include "network/network.h"

void Config::ReadMultiplayerValues() {
    qt_config->beginGroup(QStringLiteral("Multiplayer"));
    UISettings::values.nickname =
        ReadSetting(QStringLiteral("nickname"), QStringLiteral("")).toString();
    UISettings::values.ip = ReadSetting(QStringLiteral("ip"), QStringLiteral("")).toString();
    UISettings::values.port =
        ReadSetting(QStringLiteral("port"), Network::DefaultRoomPort).toString();
    UISettings::values.room_nickname =
        ReadSetting(QStringLiteral("room_nickname"), QStringLiteral("")).toString();
    UISettings::values.room_name =
        ReadSetting(QStringLiteral("room_name"), QStringLiteral("")).toString();
    UISettings::values.room_port =
        ReadSetting(QStringLiteral("room_port"), QStringLiteral("24872")).toString();
    bool ok;
    UISettings::values.host_type = ReadSetting(QStringLiteral("host_type"), 0).toUInt(&ok);
    if (!ok) {
        UISettings::values.host_type = 0;
    }
    UISettings::values.max_player = ReadSetting(QStringLiteral("max_player"), 8).toUInt();
    UISettings::values.game_id = ReadSetting(QStringLiteral("game_id"), 0).toULongLong();
    UISettings::values.room_description =
        ReadSetting(QStringLiteral("room_description"), QStringLiteral("")).toString();
    // Read ban list back
    int size = qt_config->beginReadArray(QStringLiteral("username_ban_list"));
    UISettings::values.ban_list.first.resize(size);
    for (int i = 0; i < size; ++i) {
        qt_config->setArrayIndex(i);
        UISettings::values.ban_list.first[i] =
            ReadSetting(QStringLiteral("username")).toString().toStdString();
    }
    qt_config->endArray();
    size = qt_config->beginReadArray(QStringLiteral("ip_ban_list"));
    UISettings::values.ban_list.second.resize(size);
    for (int i = 0; i < size; ++i) {
        qt_config->setArrayIndex(i);
        UISettings::values.ban_list.second[i] =
            ReadSetting(QStringLiteral("ip")).toString().toStdString();
    }
    qt_config->endArray();
    qt_config->endGroup();
}

void Config::SaveMultiplayerValues() {
    qt_config->beginGroup(QStringLiteral("Multiplayer"));
    WriteSetting(QStringLiteral("nickname"), UISettings::values.nickname, QStringLiteral(""));
    WriteSetting(QStringLiteral("ip"), UISettings::values.ip, QStringLiteral(""));
    WriteSetting(QStringLiteral("port"), UISettings::values.port, Network::DefaultRoomPort);
    WriteSetting(QStringLiteral("room_nickname"), UISettings::values.room_nickname,
                 QStringLiteral(""));
    WriteSetting(QStringLiteral("room_name"), UISettings::values.room_name, QStringLiteral(""));
    WriteSetting(QStringLiteral("room_port"), UISettings::values.room_port,
                 QStringLiteral("24872"));
    WriteSetting(QStringLiteral("host_type"), UISettings::values.host_type, 0);
    WriteSetting(QStringLiteral("max_player"), UISettings::values.max_player, 8);
    WriteSetting(QStringLiteral("game_id"), UISettings::values.game_id, 0);
    WriteSetting(QStringLiteral("room_description"), UISettings::values.room_description,
                 QStringLiteral(""));
    // Write ban list
    qt_config->beginWriteArray(QStringLiteral("username_ban_list"));
    for (std::size_t i = 0; i < UISettings::values.ban_list.first.size(); ++i) {
        qt_config->setArrayIndex(i);
        WriteSetting(QStringLiteral("username"),
                     QString::fromStdString(UISettings::values.ban_list.first[i]));
    }
    qt_config->endArray();
    qt_config->beginWriteArray(QStringLiteral("ip_ban_list"));
    for (std::size_t i = 0; i < UISettings::values.ban_list.second.size(); ++i) {
        qt_config->setArrayIndex(i);
        WriteSetting(QStringLiteral("ip"),
                     QString::fromStdString(UISettings::values.ban_list.second[i]));
    }
    qt_config->endArray();
    qt_config->endGroup();
}
