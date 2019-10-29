// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QSettings>
#include "citra_qt/configuration/config.h"

void Config::ReadCameraValues() {
    using namespace Service::CAM;
    qt_config->beginGroup("Camera");
    Settings::values.camera_name[OuterRightCamera] =
        ReadSetting("camera_outer_right_name", "blank").toString().toStdString();
    Settings::values.camera_config[OuterRightCamera] =
        ReadSetting("camera_outer_right_config", "").toString().toStdString();
    Settings::values.camera_flip[OuterRightCamera] =
        ReadSetting("camera_outer_right_flip", "0").toInt();
    Settings::values.camera_name[InnerCamera] =
        ReadSetting("camera_inner_name", "blank").toString().toStdString();
    Settings::values.camera_config[InnerCamera] =
        ReadSetting("camera_inner_config", "").toString().toStdString();
    Settings::values.camera_flip[InnerCamera] = ReadSetting("camera_inner_flip", "").toInt();
    Settings::values.camera_name[OuterLeftCamera] =
        ReadSetting("camera_outer_left_name", "blank").toString().toStdString();
    Settings::values.camera_config[OuterLeftCamera] =
        ReadSetting("camera_outer_left_config", "").toString().toStdString();
    Settings::values.camera_flip[OuterLeftCamera] =
        ReadSetting("camera_outer_left_flip", "").toInt();
    qt_config->endGroup();
}

void Config::SaveCameraValues() {
    using namespace Service::CAM;
    qt_config->beginGroup("Camera");
    WriteSetting("camera_outer_right_name",
                 QString::fromStdString(Settings::values.camera_name[OuterRightCamera]), "blank");
    WriteSetting("camera_outer_right_config",
                 QString::fromStdString(Settings::values.camera_config[OuterRightCamera]), "");
    WriteSetting("camera_outer_right_flip", Settings::values.camera_flip[OuterRightCamera], 0);
    WriteSetting("camera_inner_name",
                 QString::fromStdString(Settings::values.camera_name[InnerCamera]), "blank");
    WriteSetting("camera_inner_config",
                 QString::fromStdString(Settings::values.camera_config[InnerCamera]), "");
    WriteSetting("camera_inner_flip", Settings::values.camera_flip[InnerCamera], 0);
    WriteSetting("camera_outer_left_name",
                 QString::fromStdString(Settings::values.camera_name[OuterLeftCamera]), "blank");
    WriteSetting("camera_outer_left_config",
                 QString::fromStdString(Settings::values.camera_config[OuterLeftCamera]), "");
    WriteSetting("camera_outer_left_flip", Settings::values.camera_flip[OuterLeftCamera], 0);
    qt_config->endGroup();
}
