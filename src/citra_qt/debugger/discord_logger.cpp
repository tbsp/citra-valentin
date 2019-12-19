// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QApplication>
#include <QMessageBox>
#include <QTimer>
#include <QUrl>
#include <httplib.h>
#include "citra_qt/debugger/discord_logger.h"
#include "citra_qt/uisettings.h"
#include "citra_qt/util/discord.h"
#include "common/logging/text_formatter.h"

namespace Log {

void DiscordBackend::Write(const Entry& entry) {
    // Limit the size of the log to 7 MB
    constexpr std::size_t MAX_LOG_SIZE = 7 * 1024L * 1024L;
    if (content.size() > MAX_LOG_SIZE) {
        if (!log_too_big_in_log) {
            content.append("Log too big!");
            log_too_big_in_log = true;
        }
        return;
    }
    content += FormatLogMessage(entry).append(1, '\n');
}

void DiscordBackend::Send() {
    QTimer::singleShot(0, qApp, [=] {
        httplib::SSLClient client(
            QUrl(UISettings::values.cv_web_api_url).host().toStdString().c_str());
        httplib::Headers headers;
        headers.emplace("Authorization",
                        fmt::format("Bearer {}", UISettings::values.cv_discord_send_jwt));
        std::shared_ptr<httplib::Response> response =
            client.Post("/discord/send/log", headers, content, "text/plain");
        if (response == nullptr || response->status != 200) {
            QTimer::singleShot(0, qApp, [=] {
                if (response == nullptr) {
                    QMessageBox::critical(nullptr, QStringLiteral("Error"),
                                          QStringLiteral("Sending the log failed."));
                } else {
                    QMessageBox::critical(
                        nullptr, QStringLiteral("Error"),
                        QStringLiteral("Sending the log failed.\nStatus code: %1\n\nBody:\n%2")
                            .arg(QString::number(response->status),
                                 QString::fromStdString(response->body)));
                }
            });
        }
    });
}

} // namespace Log
