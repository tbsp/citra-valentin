// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QSettings>
#include "citra_qt/configuration/config.h"
#include "citra_qt/uisettings.h"

void Config::ReadPathsValues() {
    qt_config->beginGroup("Paths");
    UISettings::values.roms_path = ReadSetting("romsPath").toString();
    UISettings::values.symbols_path = ReadSetting("symbolsPath").toString();
    UISettings::values.movie_record_path = ReadSetting("movieRecordPath").toString();
    UISettings::values.movie_playback_path = ReadSetting("moviePlaybackPath").toString();
    UISettings::values.screenshot_path = ReadSetting("screenshotPath").toString();
    UISettings::values.video_dumping_path = ReadSetting("videoDumpingPath").toString();
    UISettings::values.game_dir_deprecated = ReadSetting("gameListRootDir", ".").toString();
    UISettings::values.game_dir_deprecated_deepscan =
        ReadSetting("gameListDeepScan", false).toBool();
    int size = qt_config->beginReadArray("gamedirs");
    for (int i = 0; i < size; ++i) {
        qt_config->setArrayIndex(i);
        UISettings::GameDir game_dir;
        game_dir.path = ReadSetting("path").toString();
        game_dir.deep_scan = ReadSetting("deep_scan", false).toBool();
        game_dir.expanded = ReadSetting("expanded", true).toBool();
        UISettings::values.game_dirs.append(game_dir);
    }
    qt_config->endArray();
    // create NAND and SD card directories if empty, these are not removable through the UI,
    // also carries over old game list settings if present
    if (UISettings::values.game_dirs.isEmpty()) {
        UISettings::GameDir game_dir;
        game_dir.path = "INSTALLED";
        game_dir.expanded = true;
        UISettings::values.game_dirs.append(game_dir);
        game_dir.path = "SYSTEM";
        UISettings::values.game_dirs.append(game_dir);
        if (UISettings::values.game_dir_deprecated != ".") {
            game_dir.path = UISettings::values.game_dir_deprecated;
            game_dir.deep_scan = UISettings::values.game_dir_deprecated_deepscan;
            UISettings::values.game_dirs.append(game_dir);
        }
    }
    UISettings::values.recent_files = ReadSetting("recentFiles").toStringList();
    qt_config->endGroup();
}

void Config::SavePathsValues() {
    qt_config->beginGroup("Paths");
    WriteSetting("romsPath", UISettings::values.roms_path);
    WriteSetting("symbolsPath", UISettings::values.symbols_path);
    WriteSetting("movieRecordPath", UISettings::values.movie_record_path);
    WriteSetting("moviePlaybackPath", UISettings::values.movie_playback_path);
    WriteSetting("screenshotPath", UISettings::values.screenshot_path);
    WriteSetting("videoDumpingPath", UISettings::values.video_dumping_path);
    qt_config->beginWriteArray("gamedirs");
    for (int i = 0; i < UISettings::values.game_dirs.size(); ++i) {
        qt_config->setArrayIndex(i);
        const auto& game_dir = UISettings::values.game_dirs[i];
        WriteSetting("path", game_dir.path);
        WriteSetting("deep_scan", game_dir.deep_scan, false);
        WriteSetting("expanded", game_dir.expanded, true);
    }
    qt_config->endArray();
    WriteSetting("recentFiles", UISettings::values.recent_files);
    qt_config->endGroup();
}
