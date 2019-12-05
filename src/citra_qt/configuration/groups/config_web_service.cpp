// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QSettings>
#include "citra_qt/configuration/config.h"
#include "citra_qt/uisettings.h"

void Config::ReadWebServiceValues() {
    qt_config->beginGroup(QStringLiteral("WebService"));
    Settings::values.web_api_url =
        ReadSetting(QStringLiteral("web_api_url"), QStringLiteral("https://api.citra-emu.org"))
            .toString()
            .toStdString();
    UISettings::values.cv_web_api_url =
        ReadSetting(QStringLiteral("cv_web_api_url"), QStringLiteral("https://cv-aadb.glitch.me"))
            .toString();
    UISettings::values.telemetry_send_os_version =
        ReadSetting(QStringLiteral("telemetry_send_os_version"), false).toBool();
    UISettings::values.telemetry_send_cpu_string =
        ReadSetting(QStringLiteral("telemetry_send_cpu_string"), false).toBool();
    UISettings::values.telemetry_send_gpu_information =
        ReadSetting(QStringLiteral("telemetry_send_gpu_information"), false).toBool();
    UISettings::values.telemetry_send_version =
        ReadSetting(QStringLiteral("telemetry_send_version"), false).toBool();
    UISettings::values.telemetry_send_citra_account_username =
        ReadSetting(QStringLiteral("telemetry_send_citra_account_username"), false).toBool();
    UISettings::values.telemetry_send_game_name =
        ReadSetting(QStringLiteral("telemetry_send_game_name"), false).toBool();
    UISettings::values.telemetry_send_use_cpu_jit =
        ReadSetting(QStringLiteral("telemetry_send_use_cpu_jit"), false).toBool();
    UISettings::values.telemetry_send_use_shader_jit =
        ReadSetting(QStringLiteral("telemetry_send_use_shader_jit"), false).toBool();
    UISettings::values.telemetry_send_use_gdbstub =
        ReadSetting(QStringLiteral("telemetry_send_use_gdbstub"), false).toBool();
    UISettings::values.telemetry_send_gdbstub_port =
        ReadSetting(QStringLiteral("telemetry_send_gdbstub_port"), false).toBool();
    UISettings::values.telemetry_send_enable_hardware_shader =
        ReadSetting(QStringLiteral("telemetry_send_enable_hardware_shader"), false).toBool();
    UISettings::values.telemetry_send_hardware_shader_accurate_multiplication =
        ReadSetting(QStringLiteral("telemetry_send_hardware_shader_accurate_multiplication"), false)
            .toBool();
    UISettings::values.telemetry_send_enable_dsp_lle =
        ReadSetting(QStringLiteral("telemetry_send_enable_dsp_lle"), false).toBool();
    UISettings::values.telemetry_send_enable_dsp_lle_multithread =
        ReadSetting(QStringLiteral("telemetry_send_enable_dsp_lle_multithread"), false).toBool();
    UISettings::values.telemetry_send_log_filter =
        ReadSetting(QStringLiteral("telemetry_send_log_filter"), false).toBool();
    Settings::values.citra_username =
        ReadSetting(QStringLiteral("citra_username")).toString().toStdString();
    Settings::values.citra_token =
        ReadSetting(QStringLiteral("citra_token")).toString().toStdString();
    qt_config->endGroup();
}

void Config::SaveWebServiceValues() {
    qt_config->beginGroup(QStringLiteral("WebService"));
    WriteSetting(QStringLiteral("web_api_url"),
                 QString::fromStdString(Settings::values.web_api_url),
                 QStringLiteral("https://api.citra-emu.org"));
    WriteSetting(QStringLiteral("cv_web_api_url"), UISettings::values.cv_web_api_url,
                 QStringLiteral("https://cv-aadb.glitch.me"));
    WriteSetting(QStringLiteral("telemetry_send_os_version"),
                 UISettings::values.telemetry_send_os_version, false);
    WriteSetting(QStringLiteral("telemetry_send_cpu_string"),
                 UISettings::values.telemetry_send_cpu_string, false);
    WriteSetting(QStringLiteral("telemetry_send_gpu_information"),
                 UISettings::values.telemetry_send_gpu_information, false);
    WriteSetting(QStringLiteral("telemetry_send_version"),
                 UISettings::values.telemetry_send_version, false);
    WriteSetting(QStringLiteral("telemetry_send_citra_account_username"),
                 UISettings::values.telemetry_send_citra_account_username, false);
    WriteSetting(QStringLiteral("telemetry_send_game_name"),
                 UISettings::values.telemetry_send_game_name, false);
    WriteSetting(QStringLiteral("telemetry_send_use_cpu_jit"),
                 UISettings::values.telemetry_send_use_cpu_jit, false);
    WriteSetting(QStringLiteral("telemetry_send_use_shader_jit"),
                 UISettings::values.telemetry_send_use_shader_jit, false);
    WriteSetting(QStringLiteral("telemetry_send_use_gdbstub"),
                 UISettings::values.telemetry_send_use_gdbstub, false);
    WriteSetting(QStringLiteral("telemetry_send_gdbstub_port"),
                 UISettings::values.telemetry_send_gdbstub_port, false);
    WriteSetting(QStringLiteral("telemetry_send_enable_hardware_shader"),
                 UISettings::values.telemetry_send_enable_hardware_shader, false);
    WriteSetting(QStringLiteral("telemetry_send_hardware_shader_accurate_multiplication"),
                 UISettings::values.telemetry_send_hardware_shader_accurate_multiplication, false);
    WriteSetting(QStringLiteral("telemetry_send_enable_dsp_lle"),
                 UISettings::values.telemetry_send_enable_dsp_lle, false);
    WriteSetting(QStringLiteral("telemetry_send_enable_dsp_lle_multithread"),
                 UISettings::values.telemetry_send_enable_dsp_lle_multithread, false);
    WriteSetting(QStringLiteral("telemetry_send_log_filter"),
                 UISettings::values.telemetry_send_log_filter, false);
    WriteSetting(QStringLiteral("citra_username"),
                 QString::fromStdString(Settings::values.citra_username));
    WriteSetting(QStringLiteral("citra_token"),
                 QString::fromStdString(Settings::values.citra_token));
    qt_config->endGroup();
}
