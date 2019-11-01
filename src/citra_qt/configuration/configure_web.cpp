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

ConfigureWeb::ConfigureWeb(QWidget* parent)
    : QWidget(parent), ui(std::make_unique<Ui::ConfigureWeb>()) {
    ui->setupUi(this);

    SetConfiguration();

    connect(ui->button_verify_login, &QPushButton::clicked, this, &ConfigureWeb::VerifyLogin);
    connect(&verify_watcher, &QFutureWatcher<bool>::finished, this, &ConfigureWeb::OnLoginVerified);

#ifndef CITRA_ENABLE_DISCORD_RP
    ui->enable_discord_rp->hide();
#endif
}

ConfigureWeb::~ConfigureWeb() = default;

void ConfigureWeb::SetConfiguration() {
#ifdef CITRA_ENABLE_DISCORD_RP
    ui->enable_discord_rp->setChecked(UISettings::values.enable_discord_rp);
#endif

    ui->web_credentials_disclaimer->setWordWrap(true);

    ui->web_signup_link->setOpenExternalLinks(true);
    ui->web_signup_link->setText(
        QStringLiteral("<a href='https://profile.citra-emu.org/'><span style=\"text-decoration: underline; "
           "color:#039be5;\">Sign up</span></a>"));
    ui->web_token_info_link->setOpenExternalLinks(true);
    ui->web_token_info_link->setText(
        QStringLiteral("<a href='https://citra-emu.org/wiki/citra-web-service/'><span style=\"text-decoration: "
           "underline; color:#039be5;\">What is my token?</span></a>"));

    ui->edit_username->setText(QString::fromStdString(Settings::values.citra_username));
    ui->edit_token->setText(QString::fromStdString(Settings::values.citra_token));
    // Connect after setting the values, to avoid calling OnLoginChanged now
    connect(ui->edit_token, &QLineEdit::textChanged, this, &ConfigureWeb::OnLoginChanged);
    connect(ui->edit_username, &QLineEdit::textChanged, this, &ConfigureWeb::OnLoginChanged);
    user_verified = true;
}

void ConfigureWeb::ApplyConfiguration() {
#ifdef CITRA_ENABLE_DISCORD_RP
    UISettings::values.enable_discord_rp = ui->enable_discord_rp->isChecked();
#endif

    if (user_verified) {
        Settings::values.citra_username = ui->edit_username->text().toStdString();
        Settings::values.citra_token = ui->edit_token->text().toStdString();
    } else {
        QMessageBox::warning(this, QStringLiteral("Username and token not verified"),
                             QStringLiteral("Username and token were not verified. The changes to your "
                                "username and/or token have not been saved."));
    }
}

void ConfigureWeb::OnLoginChanged() {
    if (ui->edit_username->text().isEmpty() && ui->edit_token->text().isEmpty()) {
        user_verified = true;

        const QPixmap pixmap = QIcon::fromTheme(QStringLiteral("checked")).pixmap(16);
        ui->label_username_verified->setPixmap(pixmap);
        ui->label_token_verified->setPixmap(pixmap);
    } else {
        user_verified = false;

        const QPixmap pixmap = QIcon::fromTheme(QStringLiteral("failed")).pixmap(16);
        ui->label_username_verified->setPixmap(pixmap);
        ui->label_token_verified->setPixmap(pixmap);
    }
}

void ConfigureWeb::VerifyLogin() {
    ui->button_verify_login->setDisabled(true);
    ui->button_verify_login->setText(QStringLiteral("Verifying..."));
    verify_watcher.setFuture(
        QtConcurrent::run([this, username = ui->edit_username->text().toStdString(),
                           token = ui->edit_token->text().toStdString()] {
            return WebService::VerifyLogin(Settings::values.web_api_url, username, token);
        }));
}

void ConfigureWeb::OnLoginVerified() {
    ui->button_verify_login->setEnabled(true);
    ui->button_verify_login->setText(QStringLiteral("Verify"));
    if (verify_watcher.result()) {
        user_verified = true;

        const QPixmap pixmap = QIcon::fromTheme(QStringLiteral("checked")).pixmap(16);
        ui->label_username_verified->setPixmap(pixmap);
        ui->label_token_verified->setPixmap(pixmap);
    } else {
        const QPixmap pixmap = QIcon::fromTheme(QStringLiteral("failed")).pixmap(16);
        ui->label_username_verified->setPixmap(pixmap);
        ui->label_token_verified->setPixmap(pixmap);
        QMessageBox::critical(
            this, QStringLiteral("Verification failed"),
            QStringLiteral("Verification failed. Check that you have entered your username and token "
               "correctly, and that your internet connection is working."));
    }
}

void ConfigureWeb::SetWebServiceConfigEnabled(bool enabled) {
    ui->label_disable_info->setVisible(!enabled);
    ui->groupBoxWebConfig->setEnabled(enabled);
}
