// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <array>
#include <string>
#include <utility>
#include <vector>
#include <QByteArray>
#include <QMetaType>
#include <QString>
#include <QStringList>
#include <QVector>
#include "common/common_types.h"

namespace UISettings {

using ContextualShortcut = std::pair<QString, int>;

struct Shortcut {
    QString name;
    QString group;
    ContextualShortcut shortcut;
};

using Themes = std::array<std::pair<const char*, const char*>, 4>;
extern const Themes themes;

struct GameDir {
    QString path;
    bool deep_scan;
    bool expanded;
    bool operator==(const GameDir& rhs) const {
        return path == rhs.path;
    };
    bool operator!=(const GameDir& rhs) const {
        return !operator==(rhs);
    };
};

enum class GameListIconSize {
    NoIcon,    ///< Do not display icons
    SmallIcon, ///< Display a small (24x24) icon
    LargeIcon, ///< Display a large (48x48) icon
};

enum class GameListText {
    NoText = -1,   ///< No text
    FileName,      ///< Display the file name of the entry
    FullPath,      ///< Display the full path of the entry
    TitleName,     ///< Display the name of the title
    TitleID,       ///< Display the title ID
    LongTitleName, ///< Display the long name of the title
    ListEnd,       ///< Keep this at the end of the enum.
};

struct Values {
    QByteArray geometry;
    QByteArray state;

    QByteArray renderwindow_geometry;

    QByteArray gamelist_header_state;

    bool single_window_mode;
    bool fullscreen;
    bool display_titlebar;
    bool show_filter_bar;
    bool show_status_bar;

    bool confirm_before_closing;
    bool pause_when_in_background;

    // Game List
    GameListIconSize game_list_icon_size;
    GameListText game_list_row_1;
    GameListText game_list_row_2;
    bool game_list_hide_no_icon;
    bool game_list_single_line_mode;

    u16 screenshot_resolution_factor;

    QString roms_path;
    QString symbols_path;
    QString movie_record_path;
    QString movie_playback_path;
    QString screenshot_path;
    QString video_dumping_path;
    QString game_dir_deprecated;
    bool game_dir_deprecated_deepscan;
    QVector<UISettings::GameDir> game_dirs;
    QStringList recent_files;

    QString theme;
    u32 callout_flags;

    std::string discord_token;

    // Qt frontend only buttons
    QString capture_screenshot_then_send_to_discord_server_button;
    QString increase_volume_button;
    QString decrease_volume_button;
    QString pause_button;

    // Shortcut name <Shortcut, context>
    std::vector<Shortcut> shortcuts;

    // Multiplayer settings
    QString nickname;
    QString ip;
    QString port;
    QString room_nickname;
    QString room_name;
    quint32 max_player;
    QString room_port;
    uint host_type;
    qulonglong game_id;
    QString room_description;
    std::pair<std::vector<std::string>, std::vector<std::string>> ban_list;

    // Logging
    bool show_console;

    // Web
    QString cv_web_api_url;
    bool telemetry_send_os_version;
    bool telemetry_send_cpu_string;
    bool telemetry_send_gpu_information;
    bool telemetry_send_version;
    bool telemetry_send_citra_account_username;
    bool telemetry_send_game_name;
    bool telemetry_send_use_cpu_jit;
    bool telemetry_send_use_shader_jit;
    bool telemetry_send_use_gdbstub;
    bool telemetry_send_gdbstub_port;
    bool telemetry_send_enable_hardware_shader;
    bool telemetry_send_hardware_shader_accurate_multiplication;
    bool telemetry_send_enable_dsp_lle;
    bool telemetry_send_enable_dsp_lle_multithread;
    bool telemetry_send_log_filter;

#ifdef CITRA_ENABLE_DISCORD_RP
    bool enable_discord_rp;
    bool discord_rp_show_game_name;
    bool discord_rp_show_room_information;
#endif
};

extern Values values;
} // namespace UISettings

Q_DECLARE_METATYPE(UISettings::GameDir*);
