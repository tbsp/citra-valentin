// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QSettings>
#include "citra_qt/configuration/config.h"
#include "common/file_util.h"

Config::Config() {
    // TODO: Don't hardcode the path; let the frontend decide where to put the config files.
    qt_config_loc = FileUtil::GetUserPath(FileUtil::UserPath::ConfigDir) + "qt-config.ini";
    FileUtil::CreateFullPath(qt_config_loc);
    qt_config =
        std::make_unique<QSettings>(QString::fromStdString(qt_config_loc), QSettings::IniFormat);
    Reload();
}

Config::~Config() {
    Save();
}

void Config::ReadValues() {
    ReadControlsValues();
    ReadCoreValues();
    ReadRendererValues();
    ReadLayoutValues();
    ReadUtilityValues();
    ReadAudioValues();
    ReadCameraValues();
    ReadDataStorageValues();
    ReadSystemValues();
    ReadMiscellaneousValues();
    ReadDebuggingValues();
    ReadWebServiceValues();
    ReadUiValues();
    ReadHacksValues();
}

void Config::SaveValues() {
    SaveControlsValues();
    SaveCoreValues();
    SaveRendererValues();
    SaveLayoutValues();
    SaveUtilityValues();
    SaveAudioValues();
    SaveCameraValues();
    SaveDataStorageValues();
    SaveSystemValues();
    SaveMiscellaneousValues();
    SaveDebuggingValues();
    SaveWebServiceValues();
    SaveUiValues();
    SaveHacksValues();
}

QVariant Config::ReadSetting(const QString& name) const {
    return qt_config->value(name);
}

QVariant Config::ReadSetting(const QString& name, const QVariant& default_value) const {
    QVariant result;
    if (qt_config->value(name + "/default", false).toBool()) {
        result = default_value;
    } else {
        result = qt_config->value(name, default_value);
    }
    return result;
}

void Config::WriteSetting(const QString& name, const QVariant& value) {
    qt_config->setValue(name, value);
}

void Config::WriteSetting(const QString& name, const QVariant& value,
                          const QVariant& default_value) {
    qt_config->setValue(name + "/default", value == default_value);
    qt_config->setValue(name, value);
}

void Config::Reload() {
    ReadValues();

    // To apply default value changes
    SaveValues();
    Settings::Apply();
}

void Config::Save() {
    SaveValues();
}
