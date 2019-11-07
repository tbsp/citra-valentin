// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QSettings>
#include "citra_qt/configuration/config.h"
#include "citra_qt/uisettings.h"

void Config::ReadPathsValues() {
    qt_config->beginGroup(QStringLiteral("Paths"));
    UISettings::values.roms_path = ReadSetting(QStringLiteral("romsPath")).toString();
    UISettings::values.symbols_path = ReadSetting(QStringLiteral("symbolsPath")).toString();
    UISettings::values.movie_record_path =
        ReadSetting(QStringLiteral("movieRecordPath")).toString();
    UISettings::values.movie_playback_path =
        ReadSetting(QStringLiteral("moviePlaybackPath")).toString();
    UISettings::values.screenshot_path = ReadSetting(QStringLiteral("screenshotPath")).toString();
    UISettings::values.video_dumping_path =
        ReadSetting(QStringLiteral("videoDumpingPath")).toString();
    UISettings::values.game_dir_deprecated =
        ReadSetting(QStringLiteral("gameListRootDir"), QStringLiteral(".")).toString();
    UISettings::values.game_dir_deprecated_deepscan =
        ReadSetting(QStringLiteral("gameListDeepScan"), false).toBool();
    int size = qt_config->beginReadArray(QStringLiteral("gamedirs"));
    for (int i = 0; i < size; ++i) {
        qt_config->setArrayIndex(i);
        UISettings::GameDir game_dir;
        game_dir.path = ReadSetting(QStringLiteral("path")).toString();
        game_dir.deep_scan = ReadSetting(QStringLiteral("deep_scan"), false).toBool();
        game_dir.expanded = ReadSetting(QStringLiteral("expanded"), true).toBool();
        UISettings::values.game_dirs.append(game_dir);
    }
    qt_config->endArray();
    // create NAND and SD card directories if empty, these are not removable through the UI,
    // also carries over old game list settings if present
    if (UISettings::values.game_dirs.isEmpty()) {
        UISettings::GameDir game_dir;
        game_dir.path = QStringLiteral("INSTALLED");
        game_dir.expanded = true;
        UISettings::values.game_dirs.append(game_dir);
        game_dir.path = QStringLiteral("SYSTEM");
        UISettings::values.game_dirs.append(game_dir);
        if (UISettings::values.game_dir_deprecated != QStringLiteral(".")) {
            game_dir.path = UISettings::values.game_dir_deprecated;
            game_dir.deep_scan = UISettings::values.game_dir_deprecated_deepscan;
            UISettings::values.game_dirs.append(game_dir);
        }
    }
    UISettings::values.recent_files = ReadSetting(QStringLiteral("recentFiles")).toStringList();
    qt_config->endGroup();
}

void Config::SavePathsValues() {
    qt_config->beginGroup(QStringLiteral("Paths"));
    WriteSetting(QStringLiteral("romsPath"), UISettings::values.roms_path);
    WriteSetting(QStringLiteral("symbolsPath"), UISettings::values.symbols_path);
    WriteSetting(QStringLiteral("movieRecordPath"), UISettings::values.movie_record_path);
    WriteSetting(QStringLiteral("moviePlaybackPath"), UISettings::values.movie_playback_path);
    WriteSetting(QStringLiteral("screenshotPath"), UISettings::values.screenshot_path);
    WriteSetting(QStringLiteral("videoDumpingPath"), UISettings::values.video_dumping_path);
    qt_config->beginWriteArray(QStringLiteral("gamedirs"));
    for (int i = 0; i < UISettings::values.game_dirs.size(); ++i) {
        qt_config->setArrayIndex(i);
        const auto& game_dir = UISettings::values.game_dirs[i];
        WriteSetting(QStringLiteral("path"), game_dir.path);
        WriteSetting(QStringLiteral("deep_scan"), game_dir.deep_scan, false);
        WriteSetting(QStringLiteral("expanded"), game_dir.expanded, true);
    }
    qt_config->endArray();
    WriteSetting(QStringLiteral("recentFiles"), UISettings::values.recent_files);
    qt_config->endGroup();
}
