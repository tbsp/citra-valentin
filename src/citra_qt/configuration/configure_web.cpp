// Copyright 2017 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QIcon>
#include <QMessageBox>
#include <QtConcurrent/QtConcurrentRun>
#include "citra_qt/configuration/configure_web.h"
#include "citra_qt/uisettings.h"
#include "core/settings.h"
#include "ui_configure_web.h"
#include "web_service/verify_login.h"

static constexpr char token_delimiter{':'};

static std::string GenerateDisplayToken(const std::string& username, const std::string& token) {
    if (username.empty() || token.empty()) {
        return {};
    }

    const std::string unencoded_display_token = username + token_delimiter + token;
    QByteArray b(unencoded_display_token.c_str());
    QByteArray b64 = b.toBase64();
    return b64.toStdString();
}

static std::string UsernameFromDisplayToken(const std::string& display_token) {
    const std::string unencoded_display_token =
        QByteArray::fromBase64(display_token.c_str()).toStdString();
    return unencoded_display_token.substr(0, unencoded_display_token.find(token_delimiter));
}

static std::string TokenFromDisplayToken(const std::string& display_token) {
    const std::string unencoded_display_token =
        QByteArray::fromBase64(display_token.c_str()).toStdString();
    return unencoded_display_token.substr(unencoded_display_token.find(token_delimiter) + 1);
}

ConfigureWeb::ConfigureWeb(QWidget* parent)
    : QWidget(parent), ui(std::make_unique<Ui::ConfigureWeb>()) {
    ui->setupUi(this);

    SetConfiguration();

    connect(ui->button_verify_login, &QPushButton::clicked, this, &ConfigureWeb::VerifyLogin);
    connect(&verify_watcher, &QFutureWatcher<bool>::finished, this, &ConfigureWeb::OnLoginVerified);

    const auto SetAllTelemetry = [this](const bool checked) {
        ui->telemetry_os_version->setChecked(checked);
        ui->telemetry_cpu_string->setChecked(checked);
        ui->telemetry_gpu_information->setChecked(checked);
        ui->telemetry_citra_valentin_version->setChecked(checked);
        ui->telemetry_citra_account_username->setChecked(checked);
        ui->telemetry_game_name->setChecked(checked);
        ui->telemetry_use_cpu_jit->setChecked(checked);
        ui->telemetry_gdb_stub_enabled->setChecked(checked);
        ui->telemetry_gdb_stub_port->setChecked(checked);
        ui->telemetry_use_hardware_shader->setChecked(checked);
        ui->telemetry_use_hardware_shader_accurate_multiplication->setChecked(checked);
        ui->telemetry_use_shader_jit->setChecked(checked);
        ui->telemetry_use_dsp_lle->setChecked(checked);
        ui->telemetry_use_dsp_lle_multi_core->setChecked(checked);
        ui->telemetry_log_filter->setChecked(checked);
    };

    connect(ui->telemetry_enable_all, &QPushButton::clicked, this,
            [SetAllTelemetry] { SetAllTelemetry(true); });
    connect(ui->telemetry_disable_all, &QPushButton::clicked, this,
            [SetAllTelemetry] { SetAllTelemetry(false); });

#ifndef CITRA_ENABLE_DISCORD_RP
    ui->enable_discord_rp->hide();
#endif
}

ConfigureWeb::~ConfigureWeb() = default;

void ConfigureWeb::SetConfiguration() {
#ifdef CITRA_ENABLE_DISCORD_RP
    ui->enable_discord_rp->setChecked(UISettings::values.enable_discord_rp);
    ui->discord_rp_show_game_name->setChecked(UISettings::values.discord_rp_show_game_name);
    ui->discord_rp_show_room_information->setChecked(
        UISettings::values.discord_rp_show_room_information);
    ui->discord_rp_show_settings->setVisible(UISettings::values.enable_discord_rp);

    connect(ui->enable_discord_rp, &QCheckBox::toggled, ui->discord_rp_show_settings,
            &QWidget::setVisible);
#endif

    ui->telemetry_os_version->setChecked(UISettings::values.telemetry_send_os_version);
    ui->telemetry_cpu_string->setChecked(UISettings::values.telemetry_send_cpu_string);
    ui->telemetry_gpu_information->setChecked(UISettings::values.telemetry_send_gpu_information);
    ui->telemetry_citra_valentin_version->setChecked(UISettings::values.telemetry_send_version);
    ui->telemetry_citra_account_username->setChecked(
        UISettings::values.telemetry_send_citra_account_username);
    ui->telemetry_game_name->setChecked(UISettings::values.telemetry_send_game_name);
    ui->telemetry_use_cpu_jit->setChecked(UISettings::values.telemetry_send_use_cpu_jit);
    ui->telemetry_gdb_stub_enabled->setChecked(UISettings::values.telemetry_send_use_gdbstub);
    ui->telemetry_gdb_stub_port->setChecked(UISettings::values.telemetry_send_gdbstub_port);
    ui->telemetry_use_hardware_shader->setChecked(
        UISettings::values.telemetry_send_enable_hardware_shader);
    ui->telemetry_use_hardware_shader_accurate_multiplication->setChecked(
        UISettings::values.telemetry_send_hardware_shader_accurate_multiplication);
    ui->telemetry_use_shader_jit->setChecked(UISettings::values.telemetry_send_use_shader_jit);
    ui->telemetry_use_dsp_lle->setChecked(UISettings::values.telemetry_send_enable_dsp_lle);
    ui->telemetry_use_dsp_lle_multi_core->setChecked(
        UISettings::values.telemetry_send_enable_dsp_lle_multithread);
    ui->telemetry_log_filter->setChecked(UISettings::values.telemetry_send_log_filter);

    ui->web_credentials_disclaimer->setWordWrap(true);

    ui->web_signup_link->setOpenExternalLinks(true);
    ui->web_signup_link->setText(QStringLiteral(
        "<a href='https://profile.citra-emu.org/'><span style=\"text-decoration: underline; "
        "color:#039be5;\">Sign up</span></a>"));
    ui->web_token_info_link->setOpenExternalLinks(true);
    ui->web_token_info_link->setText(QStringLiteral(
        "<a href='https://citra-emu.org/wiki/citra-web-service/'><span style=\"text-decoration: "
        "underline; color:#039be5;\">What is my token?</span></a>"));

    if (Settings::values.citra_username.empty()) {
        ui->username->setText(QStringLiteral("Unspecified"));
    } else {
        ui->username->setText(QString::fromStdString(Settings::values.citra_username));
    }

    ui->edit_token->setText(QString::fromStdString(
        GenerateDisplayToken(Settings::values.citra_username, Settings::values.citra_token)));

    // Connect after setting the values, to avoid calling OnLoginChanged now
    connect(ui->edit_token, &QLineEdit::textChanged, this, &ConfigureWeb::OnLoginChanged);
    user_verified = true;
}

void ConfigureWeb::ApplyConfiguration() {
#ifdef CITRA_ENABLE_DISCORD_RP
    UISettings::values.enable_discord_rp = ui->enable_discord_rp->isChecked();
    UISettings::values.discord_rp_show_game_name = ui->discord_rp_show_game_name->isChecked();
    UISettings::values.discord_rp_show_room_information =
        ui->discord_rp_show_room_information->isChecked();
#endif

    UISettings::values.telemetry_send_os_version = ui->telemetry_os_version->isChecked();
    UISettings::values.telemetry_send_cpu_string = ui->telemetry_cpu_string->isChecked();
    UISettings::values.telemetry_send_gpu_information = ui->telemetry_gpu_information->isChecked();
    UISettings::values.telemetry_send_version = ui->telemetry_citra_valentin_version->isChecked();
    UISettings::values.telemetry_send_citra_account_username =
        ui->telemetry_citra_account_username->isChecked();
    UISettings::values.telemetry_send_game_name = ui->telemetry_game_name->isChecked();
    UISettings::values.telemetry_send_use_cpu_jit = ui->telemetry_use_cpu_jit->isChecked();
    UISettings::values.telemetry_send_use_gdbstub = ui->telemetry_gdb_stub_enabled->isChecked();
    UISettings::values.telemetry_send_gdbstub_port = ui->telemetry_gdb_stub_port->isChecked();
    UISettings::values.telemetry_send_enable_hardware_shader =
        ui->telemetry_use_hardware_shader->isChecked();
    UISettings::values.telemetry_send_hardware_shader_accurate_multiplication =
        ui->telemetry_use_hardware_shader_accurate_multiplication->isChecked();
    UISettings::values.telemetry_send_use_shader_jit = ui->telemetry_use_shader_jit->isChecked();
    UISettings::values.telemetry_send_enable_dsp_lle = ui->telemetry_use_dsp_lle->isChecked();
    UISettings::values.telemetry_send_enable_dsp_lle_multithread =
        ui->telemetry_use_dsp_lle_multi_core->isChecked();
    UISettings::values.telemetry_send_log_filter = ui->telemetry_log_filter->isChecked();

    if (user_verified) {
        Settings::values.citra_username =
            UsernameFromDisplayToken(ui->edit_token->text().toStdString());
        Settings::values.citra_token = TokenFromDisplayToken(ui->edit_token->text().toStdString());
    } else {
        QMessageBox::warning(
            this, QStringLiteral("Token not verified"),
            QStringLiteral("Token was not verified. The change to your token has not been saved."));
    }
}

void ConfigureWeb::OnLoginChanged() {
    if (ui->edit_token->text().isEmpty()) {
        user_verified = true;

        const QPixmap pixmap = QIcon::fromTheme(QStringLiteral("checked")).pixmap(16);
        ui->label_token_verified->setPixmap(pixmap);
    } else {
        user_verified = false;

        const QPixmap pixmap = QIcon::fromTheme(QStringLiteral("failed")).pixmap(16);
        ui->label_token_verified->setPixmap(pixmap);
    }
}

void ConfigureWeb::VerifyLogin() {
    ui->button_verify_login->setDisabled(true);
    ui->button_verify_login->setText(tr("Verifying..."));
    verify_watcher.setFuture(QtConcurrent::run(
        [username = UsernameFromDisplayToken(ui->edit_token->text().toStdString()),
         token = TokenFromDisplayToken(ui->edit_token->text().toStdString())] {
            return WebService::VerifyLogin(Settings::values.web_api_url, username, token);
        }));
}

void ConfigureWeb::OnLoginVerified() {
    ui->button_verify_login->setEnabled(true);
    ui->button_verify_login->setText(QStringLiteral("Verify"));
    if (verify_watcher.result()) {
        user_verified = true;

        const QPixmap pixmap = QIcon::fromTheme(QStringLiteral("checked")).pixmap(16);
        ui->label_token_verified->setPixmap(pixmap);
        ui->username->setText(
            QString::fromStdString(UsernameFromDisplayToken(ui->edit_token->text().toStdString())));
    } else {
        const QPixmap pixmap = QIcon::fromTheme(QStringLiteral("failed")).pixmap(16);
        ui->label_token_verified->setPixmap(pixmap);
        ui->username->setText(QStringLiteral("Unspecified"));
        QMessageBox::critical(
            this, QStringLiteral("Verification failed"),
            QStringLiteral("Verification failed. Check that you have entered your token "
                           "correctly, and that your internet connection is working."));
    }
}

void ConfigureWeb::SetWebServiceConfigEnabled(bool enabled) {
    ui->label_disable_info->setVisible(!enabled);
    ui->groupBoxWebConfig->setEnabled(enabled);
}
