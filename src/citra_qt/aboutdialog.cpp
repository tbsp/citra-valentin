// Copyright 2017 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QIcon>
#include "aboutdialog.h"
#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget* parent)
    : QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowSystemMenuHint),
      ui(new Ui::AboutDialog) {
    ui->setupUi(this);
    ui->labelLogo->setPixmap(QIcon::fromTheme(QStringLiteral("citra")).pixmap(200));
}

AboutDialog::~AboutDialog() = default;
