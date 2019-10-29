// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <array>
#include <memory>
#include <string>
#include <QVariant>
#include "core/settings.h"

class QSettings;

class Config {
public:
    Config();
    ~Config();

    void Reload();
    void Save();

    static const std::array<int, Settings::NativeButton::NumButtons> default_buttons;
    static const std::array<std::array<int, 5>, Settings::NativeAnalog::NumAnalogs> default_analogs;

private:
    void ReadValues();
    void ReadControlsValues();
    void ReadCoreValues();
    void ReadRendererValues();
    void ReadLayoutValues();
    void ReadUtilityValues();
    void ReadAudioValues();
    void ReadCameraValues();
    void ReadDataStorageValues();
    void ReadSystemValues();
    void ReadMiscellaneousValues();
    void ReadDebuggingValues();
    void ReadLleValues();
    void ReadWebServiceValues();
    void ReadUiValues();
    void ReadUiLayoutValues();
    void ReadGameListValues();
    void ReadPathsValues();
    void ReadShortcutsValues();
    void ReadMultiplayerValues();
    void ReadHacksValues();

    void SaveValues();
    void SaveControlsValues();
    void SaveCoreValues();
    void SaveRendererValues();
    void SaveLayoutValues();
    void SaveUtilityValues();
    void SaveAudioValues();
    void SaveCameraValues();
    void SaveDataStorageValues();
    void SaveSystemValues();
    void SaveMiscellaneousValues();
    void SaveDebuggingValues();
    void SaveLleValues();
    void SaveWebServiceValues();
    void SaveUiValues();
    void SaveUiLayoutValues();
    void SaveGameListValues();
    void SavePathsValues();
    void SaveShortcutsValues();
    void SaveMultiplayerValues();
    void SaveHacksValues();

    QVariant ReadSetting(const QString& name) const;
    QVariant ReadSetting(const QString& name, const QVariant& default_value) const;
    void WriteSetting(const QString& name, const QVariant& value);
    void WriteSetting(const QString& name, const QVariant& value, const QVariant& default_value);

    std::unique_ptr<QSettings> qt_config;
    std::string qt_config_loc;
};
