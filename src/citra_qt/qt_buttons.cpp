// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <chrono>
#include <QTimer>
#include "citra_qt/qt_buttons.h"
#include "citra_qt/uisettings.h"
#include "core/frontend/input.h"
#include "core/settings.h"

QtButtons::QtButtons() {
    Update();

    timer = std::make_unique<QTimer>();

    using namespace std::chrono_literals;
    timer->setInterval(10ms);

    connect(timer.get(), &QTimer::timeout, this, [=] {
        if (capture_screenshot_then_send_to_discord_server_button->GetStatus()) {
            emit CaptureScreenshotThenSendToDiscordServerRequested();
        }

        if (increase_volume_button->GetStatus() && Settings::values.volume < 1.0f) {
            Settings::values.volume += 0.01f;
        }

        if (decrease_volume_button->GetStatus() && Settings::values.volume >= 0.01f) {
            Settings::values.volume -= 0.01f;
        }
    });

    timer->start();
}

QtButtons::~QtButtons() = default;

void QtButtons::Update() {
    capture_screenshot_then_send_to_discord_server_button =
        Input::CreateDevice<Input::ButtonDevice>(
            UISettings::values.capture_screenshot_then_send_to_discord_server_button.toStdString());
    increase_volume_button = Input::CreateDevice<Input::ButtonDevice>(
        UISettings::values.increase_volume_button.toStdString());
    decrease_volume_button = Input::CreateDevice<Input::ButtonDevice>(
        UISettings::values.decrease_volume_button.toStdString());
}
