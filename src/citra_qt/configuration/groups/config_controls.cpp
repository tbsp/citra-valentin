// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QSettings>
#include "citra_qt/configuration/config.h"
#include "input_common/main.h"
#include "input_common/udp/client.h"

const std::array<int, Settings::NativeButton::NumButtons> Config::default_buttons = {
    Qt::Key_A, Qt::Key_S, Qt::Key_Z, Qt::Key_X, Qt::Key_T, Qt::Key_G,
    Qt::Key_F, Qt::Key_H, Qt::Key_Q, Qt::Key_W, Qt::Key_M, Qt::Key_N,
    Qt::Key_O, Qt::Key_P, Qt::Key_1, Qt::Key_2, Qt::Key_B,
};

const std::array<std::array<int, 5>, Settings::NativeAnalog::NumAnalogs> Config::default_analogs{{
    {
        Qt::Key_Up,
        Qt::Key_Down,
        Qt::Key_Left,
        Qt::Key_Right,
        Qt::Key_D,
    },
    {
        Qt::Key_I,
        Qt::Key_K,
        Qt::Key_J,
        Qt::Key_L,
        Qt::Key_D,
    },
}};

void Config::ReadControlsValues() {
    qt_config->beginGroup("Controls");

    Settings::values.current_input_profile_index = ReadSetting("profile", 0).toInt();

    const auto append_profile = [this] {
        Settings::InputProfile profile;
        profile.name = ReadSetting("name", "default").toString().toStdString();
        for (int i = 0; i < Settings::NativeButton::NumButtons; ++i) {
            std::string default_param = InputCommon::GenerateKeyboardParam(default_buttons[i]);
            profile.buttons[i] = ReadSetting(Settings::NativeButton::mapping[i],
                                             QString::fromStdString(default_param))
                                     .toString()
                                     .toStdString();
            if (profile.buttons[i].empty())
                profile.buttons[i] = default_param;
        }
        for (int i = 0; i < Settings::NativeAnalog::NumAnalogs; ++i) {
            std::string default_param = InputCommon::GenerateAnalogParamFromKeys(
                default_analogs[i][0], default_analogs[i][1], default_analogs[i][2],
                default_analogs[i][3], default_analogs[i][4], 0.5f);
            profile.analogs[i] = ReadSetting(Settings::NativeAnalog::mapping[i],
                                             QString::fromStdString(default_param))
                                     .toString()
                                     .toStdString();
            if (profile.analogs[i].empty())
                profile.analogs[i] = default_param;
        }
        profile.motion_device =
            ReadSetting("motion_device",
                        "engine:motion_emu,update_period:100,sensitivity:0.01,tilt_clamp:90.0")
                .toString()
                .toStdString();
        profile.touch_device =
            ReadSetting("touch_device", "engine:emu_window").toString().toStdString();
        profile.udp_input_address =
            ReadSetting("udp_input_address", InputCommon::CemuhookUDP::DEFAULT_ADDR)
                .toString()
                .toStdString();
        profile.udp_input_port = static_cast<u16>(
            ReadSetting("udp_input_port", InputCommon::CemuhookUDP::DEFAULT_PORT).toInt());
        profile.udp_pad_index = static_cast<u8>(ReadSetting("udp_pad_index", 0).toUInt());
        Settings::values.input_profiles.emplace_back(std::move(profile));
    };

    int num_input_profiles = qt_config->beginReadArray("profiles");

    for (int i = 0; i < num_input_profiles; ++i) {
        qt_config->setArrayIndex(i);
        append_profile();
    }

    qt_config->endArray();

    // create a input profile if no input profiles exist, with the default or old settings
    if (num_input_profiles == 0) {
        append_profile();
        num_input_profiles = 1;
    }

    // ensure that the current input profile index is valid.
    Settings::values.current_input_profile_index =
        std::clamp(Settings::values.current_input_profile_index, 0, num_input_profiles - 1);

    Settings::LoadProfile(Settings::values.current_input_profile_index);

    qt_config->endGroup();
}

void Config::SaveControlsValues() {
    qt_config->beginGroup("Controls");
    WriteSetting("profile", Settings::values.current_input_profile_index, 0);
    qt_config->beginWriteArray("profiles");
    for (std::size_t p = 0; p < Settings::values.input_profiles.size(); ++p) {
        qt_config->setArrayIndex(static_cast<int>(p));
        const auto& profile = Settings::values.input_profiles[p];
        WriteSetting("name", QString::fromStdString(profile.name), "default");
        for (int i = 0; i < Settings::NativeButton::NumButtons; ++i) {
            std::string default_param = InputCommon::GenerateKeyboardParam(default_buttons[i]);
            WriteSetting(QString::fromStdString(Settings::NativeButton::mapping[i]),
                         QString::fromStdString(profile.buttons[i]),
                         QString::fromStdString(default_param));
        }
        for (int i = 0; i < Settings::NativeAnalog::NumAnalogs; ++i) {
            std::string default_param = InputCommon::GenerateAnalogParamFromKeys(
                default_analogs[i][0], default_analogs[i][1], default_analogs[i][2],
                default_analogs[i][3], default_analogs[i][4], 0.5f);
            WriteSetting(QString::fromStdString(Settings::NativeAnalog::mapping[i]),
                         QString::fromStdString(profile.analogs[i]),
                         QString::fromStdString(default_param));
        }
        WriteSetting("motion_device", QString::fromStdString(profile.motion_device),
                     "engine:motion_emu,update_period:100,sensitivity:0.01,tilt_clamp:90.0");
        WriteSetting("touch_device", QString::fromStdString(profile.touch_device),
                     "engine:emu_window");
        WriteSetting("udp_input_address", QString::fromStdString(profile.udp_input_address),
                     InputCommon::CemuhookUDP::DEFAULT_ADDR);
        WriteSetting("udp_input_port", profile.udp_input_port,
                     InputCommon::CemuhookUDP::DEFAULT_PORT);
        WriteSetting("udp_pad_index", profile.udp_pad_index, 0);
    }
    qt_config->endArray();
    qt_config->endGroup();
}
