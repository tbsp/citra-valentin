// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QDesktopServices>
#include <QMessageBox>
#include <QUrl>
#include "citra_qt/configuration/configure_general.h"
#include "citra_qt/uisettings.h"
#include "core/core.h"
#include "core/settings.h"
#include "ui_configure_general.h"

// The QSlider doesn't have an easy way to set a custom step amount,
// so we can just convert from the sliders range (0 - 75) to the expected
// settings range (25 - 400) with simple math.
static constexpr int SliderToSettings(int value) {
    return 5 * value + 25;
}

static constexpr int SettingsToSlider(int value) {
    return (value - 25) / 5;
}

ConfigureGeneral::ConfigureGeneral(QWidget* parent)
    : QWidget(parent), ui(new Ui::ConfigureGeneral) {

    ui->setupUi(this);
    SetConfiguration();

    connect(ui->toggle_frame_limit, &QCheckBox::toggled, ui->frame_limit, &QSpinBox::setVisible);
    connect(ui->button_reset_defaults, &QPushButton::clicked, this,
            &ConfigureGeneral::ResetDefaults);

    connect(ui->button_open_configuration_location, &QPushButton::clicked, this, [] {
        const QString path =
            QString::fromStdString(FileUtil::GetUserPath(FileUtil::UserPath::ConfigDir));
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    });

    connect(ui->slider_clock_speed, &QSlider::valueChanged, [&](int value) {
        ui->clock_display_label->setText(QStringLiteral("%1%").arg(SliderToSettings(value)));
    });

    connect(ui->use_custom_cpu_ticks, &QCheckBox::toggled, ui->custom_cpu_ticks,
            &QSpinBox::setEnabled);
}

ConfigureGeneral::~ConfigureGeneral() = default;

void ConfigureGeneral::SetConfiguration() {
    // The first item is "auto-select" with actual value -1, so plus one here will do the trick
    ui->region_combobox->setCurrentIndex(Settings::values.region_value + 1);

    ui->toggle_frame_limit->setChecked(Settings::values.use_frame_limit);
    ui->frame_limit->setVisible(ui->toggle_frame_limit->isChecked());
    ui->frame_limit->setValue(Settings::values.frame_limit);
    ui->toggle_check_exit->setChecked(UISettings::values.confirm_before_closing);
    ui->toggle_background_pause->setChecked(UISettings::values.pause_when_in_background);
    ui->use_custom_cpu_ticks->setChecked(Settings::values.use_custom_cpu_ticks);
    ui->custom_cpu_ticks->setEnabled(Settings::values.use_custom_cpu_ticks);
    ui->custom_cpu_ticks->setValue(Settings::values.custom_cpu_ticks);
    ui->slider_clock_speed->setValue(SettingsToSlider(Settings::values.cpu_clock_percentage));
    ui->clock_display_label->setText(QString("%1%").arg(Settings::values.cpu_clock_percentage));
}

void ConfigureGeneral::ResetDefaults() {
    const QMessageBox::StandardButton answer = QMessageBox::question(
        this, QStringLiteral("Citra Valentin"),
        QStringLiteral(
            "Are you sure you want to <b>reset your settings</b> and close Citra Valentin?"),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (answer == QMessageBox::No) {
        return;
    }

    FileUtil::Delete(FileUtil::GetUserPath(FileUtil::UserPath::ConfigDir) + "qt-config.ini");
    std::exit(0);
}

void ConfigureGeneral::ApplyConfiguration() {
    Settings::values.use_frame_limit = ui->toggle_frame_limit->isChecked();
    Settings::values.frame_limit = ui->frame_limit->value();
    UISettings::values.confirm_before_closing = ui->toggle_check_exit->isChecked();
    UISettings::values.pause_when_in_background = ui->toggle_background_pause->isChecked();
    Settings::values.region_value = ui->region_combobox->currentIndex() - 1;
    Settings::values.use_frame_limit = ui->toggle_frame_limit->isChecked();
    Settings::values.frame_limit = ui->frame_limit->value();
    Settings::values.use_custom_cpu_ticks = ui->use_custom_cpu_ticks->isChecked();
    Settings::values.custom_cpu_ticks = ui->custom_cpu_ticks->value();
    Settings::values.cpu_clock_percentage = SliderToSettings(ui->slider_clock_speed->value());
}
