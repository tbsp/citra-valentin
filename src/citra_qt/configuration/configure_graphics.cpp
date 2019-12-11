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
            &QWidget::setVisible);

    ui->sharper_distant_objects->setDisabled(Core::System::GetInstance().IsPoweredOn());

    ui->custom_screen_refresh_rate->setVisible(ui->toggle_custom_screen_refresh_rate->isChecked());
    connect(ui->toggle_custom_screen_refresh_rate, &QCheckBox::toggled,
            ui->custom_screen_refresh_rate, &QSpinBox::setVisible);

    ui->hw_shader_group->setVisible(ui->toggle_hw_shader->isChecked());
    connect(ui->toggle_hw_shader, &QCheckBox::toggled, ui->hw_shader_group, &QWidget::setVisible);
}

ConfigureGraphics::~ConfigureGraphics() = default;

void ConfigureGraphics::SetConfiguration() {
    ui->toggle_hw_renderer->setChecked(Settings::values.use_hw_renderer);
    ui->toggle_hw_shader->setChecked(Settings::values.use_hw_shader);
    ui->toggle_accurate_mul->setChecked(Settings::values.shaders_accurate_mul);
    ui->toggle_shader_jit->setChecked(Settings::values.use_shader_jit);
    ui->toggle_disk_cache->setChecked(Settings::values.enable_disk_shader_cache);
    ui->sharper_distant_objects->setChecked(Settings::values.sharper_distant_objects);
    ui->ignore_format_reinterpretation->setChecked(Settings::values.ignore_format_reinterpretation);
    ui->toggle_custom_screen_refresh_rate->setChecked(Settings::values.custom_screen_refresh_rate);
    ui->custom_screen_refresh_rate->setValue(Settings::values.screen_refresh_rate);
    ui->min_vertices_per_thread->setValue(Settings::values.min_vertices_per_thread);
}

void ConfigureGraphics::ApplyConfiguration() {
    Settings::values.use_hw_renderer = ui->toggle_hw_renderer->isChecked();
    Settings::values.use_hw_shader = ui->toggle_hw_shader->isChecked();
    Settings::values.shaders_accurate_mul = ui->toggle_accurate_mul->isChecked();
    Settings::values.use_shader_jit = ui->toggle_shader_jit->isChecked();
    Settings::values.enable_disk_shader_cache = ui->toggle_disk_cache->isChecked();
    Settings::values.sharper_distant_objects = ui->sharper_distant_objects->isChecked();
    Settings::values.ignore_format_reinterpretation =
        ui->ignore_format_reinterpretation->isChecked();
    Settings::values.custom_screen_refresh_rate =
        ui->toggle_custom_screen_refresh_rate->isChecked();
    Settings::values.screen_refresh_rate = ui->custom_screen_refresh_rate->value();
    Settings::values.min_vertices_per_thread = ui->min_vertices_per_thread->value();
}
