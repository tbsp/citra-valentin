// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <QObject>
#include "core/frontend/input.h"

class QTimer;

class QtButtons : public QObject {
    Q_OBJECT

public:
    QtButtons();
    ~QtButtons();

    void Update();

private:
    std::unique_ptr<QTimer> timer;
    std::unique_ptr<Input::ButtonDevice> capture_screenshot_then_send_to_discord_server_button;
    std::unique_ptr<Input::ButtonDevice> increase_volume_button;
    std::unique_ptr<Input::ButtonDevice> decrease_volume_button;

signals:
    void CaptureScreenshotThenSendToDiscordServerRequested();
};
