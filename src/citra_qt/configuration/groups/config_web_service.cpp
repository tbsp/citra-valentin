// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QSettings>
#include "citra_qt/configuration/config.h"

void Config::ReadWebServiceValues() {
    qt_config->beginGroup("WebService");
    Settings::values.web_api_url =
        ReadSetting("web_api_url", "https://api.citra-emu.org").toString().toStdString();
    Settings::values.citra_username = ReadSetting("citra_username").toString().toStdString();
    Settings::values.citra_token = ReadSetting("citra_token").toString().toStdString();
    qt_config->endGroup();
}

void Config::SaveWebServiceValues() {
    qt_config->beginGroup("WebService");
    WriteSetting("web_api_url", QString::fromStdString(Settings::values.web_api_url),
                 "https://api.citra-emu.org");
    WriteSetting("citra_username", QString::fromStdString(Settings::values.citra_username));
    WriteSetting("citra_token", QString::fromStdString(Settings::values.citra_token));
    qt_config->endGroup();
}
