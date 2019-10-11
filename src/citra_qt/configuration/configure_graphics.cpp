// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QColorDialog>
#include "citra_qt/configuration/configure_graphics.h"
#include "core/core.h"
#include "core/settings.h"
#include "ui_configure_graphics.h"
#include "video_core/renderer_opengl/post_processing_opengl.h"

ConfigureGraphics::ConfigureGraphics(QWidget* parent)
    : QWidget(parent), ui(new Ui::ConfigureGraphics) {
    ui->setupUi(this);
    SetConfiguration();

    ui->hw_renderer_group->setVisible(ui->toggle_hw_renderer->isChecked());
    connect(ui->toggle_hw_renderer, &QCheckBox::toggled, ui->hw_renderer_group,
            &QGroupBox::setVisible);

    ui->hw_shader_group->setVisible(ui->toggle_hw_shader->isChecked());
    connect(ui->toggle_hw_shader, &QCheckBox::toggled, ui->hw_shader_group, &QGroupBox::setVisible);
}

ConfigureGraphics::~ConfigureGraphics() = default;

void ConfigureGraphics::SetConfiguration() {
    ui->toggle_hw_renderer->setChecked(Settings::values.use_hw_renderer);
    ui->toggle_hw_shader->setChecked(Settings::values.use_hw_shader);
    ui->toggle_accurate_mul->setChecked(Settings::values.shaders_accurate_mul);
    ui->toggle_shader_jit->setChecked(Settings::values.use_shader_jit);
    ui->toggle_disk_cache->setChecked(Settings::values.use_disk_shader_cache);
}

void ConfigureGraphics::ApplyConfiguration() {
    Settings::values.use_hw_renderer = ui->toggle_hw_renderer->isChecked();
    Settings::values.use_hw_shader = ui->toggle_hw_shader->isChecked();
    Settings::values.shaders_accurate_mul = ui->toggle_accurate_mul->isChecked();
    Settings::values.use_shader_jit = ui->toggle_shader_jit->isChecked();
    Settings::values.use_disk_shader_cache = ui->toggle_disk_cache->isChecked();
}
