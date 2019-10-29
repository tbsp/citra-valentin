// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QSettings>
#include "citra_qt/configuration/config.h"
#include "citra_qt/uisettings.h"
#include "network/network.h"

void Config::ReadMultiplayerValues() {
    qt_config->beginGroup("Multiplayer");
    UISettings::values.nickname = ReadSetting("nickname", "").toString();
    UISettings::values.ip = ReadSetting("ip", "").toString();
    UISettings::values.port = ReadSetting("port", Network::DefaultRoomPort).toString();
    UISettings::values.room_nickname = ReadSetting("room_nickname", "").toString();
    UISettings::values.room_name = ReadSetting("room_name", "").toString();
    UISettings::values.room_port = ReadSetting("room_port", "24872").toString();
    bool ok;
    UISettings::values.host_type = ReadSetting("host_type", 0).toUInt(&ok);
    if (!ok) {
        UISettings::values.host_type = 0;
    }
    UISettings::values.max_player = ReadSetting("max_player", 8).toUInt();
    UISettings::values.game_id = ReadSetting("game_id", 0).toULongLong();
    UISettings::values.room_description = ReadSetting("room_description", "").toString();
    // Read ban list back
    int size = qt_config->beginReadArray("username_ban_list");
    UISettings::values.ban_list.first.resize(size);
    for (int i = 0; i < size; ++i) {
        qt_config->setArrayIndex(i);
        UISettings::values.ban_list.first[i] = ReadSetting("username").toString().toStdString();
    }
    qt_config->endArray();
    size = qt_config->beginReadArray("ip_ban_list");
    UISettings::values.ban_list.second.resize(size);
    for (int i = 0; i < size; ++i) {
        qt_config->setArrayIndex(i);
        UISettings::values.ban_list.second[i] = ReadSetting("ip").toString().toStdString();
    }
    qt_config->endArray();
    qt_config->endGroup();
}

void Config::SaveMultiplayerValues() {
    qt_config->beginGroup("Multiplayer");
    WriteSetting("nickname", UISettings::values.nickname, "");
    WriteSetting("ip", UISettings::values.ip, "");
    WriteSetting("port", UISettings::values.port, Network::DefaultRoomPort);
    WriteSetting("room_nickname", UISettings::values.room_nickname, "");
    WriteSetting("room_name", UISettings::values.room_name, "");
    WriteSetting("room_port", UISettings::values.room_port, "24872");
    WriteSetting("host_type", UISettings::values.host_type, 0);
    WriteSetting("max_player", UISettings::values.max_player, 8);
    WriteSetting("game_id", UISettings::values.game_id, 0);
    WriteSetting("room_description", UISettings::values.room_description, "");
    // Write ban list
    qt_config->beginWriteArray("username_ban_list");
    for (std::size_t i = 0; i < UISettings::values.ban_list.first.size(); ++i) {
        qt_config->setArrayIndex(i);
        WriteSetting("username", QString::fromStdString(UISettings::values.ban_list.first[i]));
    }
    qt_config->endArray();
    qt_config->beginWriteArray("ip_ban_list");
    for (std::size_t i = 0; i < UISettings::values.ban_list.second.size(); ++i) {
        qt_config->setArrayIndex(i);
        WriteSetting("ip", QString::fromStdString(UISettings::values.ban_list.second[i]));
    }
    qt_config->endArray();
    qt_config->endGroup();
}
