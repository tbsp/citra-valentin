// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QSettings>
#include "citra_qt/configuration/config.h"

void Config::ReadCameraValues() {
    using namespace Service::CAM;
    qt_config->beginGroup(QStringLiteral("Camera"));
    Settings::values.camera_name[OuterRightCamera] =
        ReadSetting(QStringLiteral("camera_outer_right_name"), QStringLiteral("blank"))
            .toString()
            .toStdString();
    Settings::values.camera_config[OuterRightCamera] =
        ReadSetting(QStringLiteral("camera_outer_right_config"), QStringLiteral(""))
            .toString()
            .toStdString();
    Settings::values.camera_flip[OuterRightCamera] =
        ReadSetting(QStringLiteral("camera_outer_right_flip"), QStringLiteral("0")).toInt();
    Settings::values.camera_name[InnerCamera] =
        ReadSetting(QStringLiteral("camera_inner_name"), QStringLiteral("blank"))
            .toString()
            .toStdString();
    Settings::values.camera_config[InnerCamera] =
        ReadSetting(QStringLiteral("camera_inner_config"), QStringLiteral(""))
            .toString()
            .toStdString();
    Settings::values.camera_flip[InnerCamera] =
        ReadSetting(QStringLiteral("camera_inner_flip"), QStringLiteral("")).toInt();
    Settings::values.camera_name[OuterLeftCamera] =
        ReadSetting(QStringLiteral("camera_outer_left_name"), QStringLiteral("blank"))
            .toString()
            .toStdString();
    Settings::values.camera_config[OuterLeftCamera] =
        ReadSetting(QStringLiteral("camera_outer_left_config"), QStringLiteral(""))
            .toString()
            .toStdString();
    Settings::values.camera_flip[OuterLeftCamera] =
        ReadSetting(QStringLiteral("camera_outer_left_flip"), QStringLiteral("")).toInt();
    qt_config->endGroup();
}

void Config::SaveCameraValues() {
    using namespace Service::CAM;
    qt_config->beginGroup(QStringLiteral("Camera"));
    WriteSetting(QStringLiteral("camera_outer_right_name"),
                 QString::fromStdString(Settings::values.camera_name[OuterRightCamera]),
                 QStringLiteral("blank"));
    WriteSetting(QStringLiteral("camera_outer_right_config"),
                 QString::fromStdString(Settings::values.camera_config[OuterRightCamera]),
                 QStringLiteral(""));
    WriteSetting(QStringLiteral("camera_outer_right_flip"),
                 Settings::values.camera_flip[OuterRightCamera], 0);
    WriteSetting(QStringLiteral("camera_inner_name"),
                 QString::fromStdString(Settings::values.camera_name[InnerCamera]),
                 QStringLiteral("blank"));
    WriteSetting(QStringLiteral("camera_inner_config"),
                 QString::fromStdString(Settings::values.camera_config[InnerCamera]),
                 QStringLiteral(""));
    WriteSetting(QStringLiteral("camera_inner_flip"), Settings::values.camera_flip[InnerCamera], 0);
    WriteSetting(QStringLiteral("camera_outer_left_name"),
                 QString::fromStdString(Settings::values.camera_name[OuterLeftCamera]),
                 QStringLiteral("blank"));
    WriteSetting(QStringLiteral("camera_outer_left_config"),
                 QString::fromStdString(Settings::values.camera_config[OuterLeftCamera]),
                 QStringLiteral(""));
    WriteSetting(QStringLiteral("camera_outer_left_flip"),
                 Settings::values.camera_flip[OuterLeftCamera], 0);
    qt_config->endGroup();
}
