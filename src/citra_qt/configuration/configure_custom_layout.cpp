// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "citra_qt/configuration/configure_custom_layout.h"
#include "core/settings.h"
#include "ui_configure_custom_layout.h"
#include "video_core/renderer_opengl/post_processing_opengl.h"

ConfigureCustomLayout::ConfigureCustomLayout(QWidget* parent)
    : QWidget(parent), ui(new Ui::ConfigureCustomLayout) {
    ui->setupUi(this);
    SetConfiguration();

    connect(ui->enabled, &QCheckBox::toggled, ui->screens, &QWidget::setVisible);
}

void ConfigureCustomLayout::SetConfiguration() {
    ui->enabled->setChecked(Settings::values.custom_layout);
    ui->screens->setVisible(Settings::values.custom_layout);
    ui->top_left->setValue(Settings::values.custom_top_left);
    ui->top_top->setValue(Settings::values.custom_top_top);
    ui->top_right->setValue(Settings::values.custom_top_right);
    ui->top_bottom->setValue(Settings::values.custom_top_bottom);
    ui->bottom_left->setValue(Settings::values.custom_bottom_left);
    ui->bottom_top->setValue(Settings::values.custom_bottom_top);
    ui->bottom_right->setValue(Settings::values.custom_bottom_right);
    ui->bottom_bottom->setValue(Settings::values.custom_bottom_bottom);
}

void ConfigureCustomLayout::ApplyConfiguration() {
    Settings::values.custom_layout = ui->enabled->isChecked();
    Settings::values.custom_top_left = ui->top_left->value();
    Settings::values.custom_top_top = ui->top_top->value();
    Settings::values.custom_top_right = ui->top_right->value();
    Settings::values.custom_top_bottom = ui->top_bottom->value();
    Settings::values.custom_bottom_left = ui->bottom_left->value();
    Settings::values.custom_bottom_top = ui->bottom_top->value();
    Settings::values.custom_bottom_right = ui->bottom_right->value();
    Settings::values.custom_bottom_bottom = ui->bottom_bottom->value();
}

ConfigureCustomLayout::~ConfigureCustomLayout() {
    delete ui;
}
