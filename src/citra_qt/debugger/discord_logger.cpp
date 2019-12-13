// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QApplication>
#include <QTimer>
#include <httplib.h>
#include "citra_qt/debugger/discord_logger.h"
#include "citra_qt/util/discord.h"
#include "common/logging/text_formatter.h"

namespace Log {

void DiscordBackend::Write(const Entry& entry) {
    // Discord limits the size of the files to 8MB
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
        nlohmann::json json = DiscordUtil::GetBaseJson();
        json["file"] = "citra-valentin-qt.log";

        const std::string boundary = httplib::detail::make_multipart_data_boundary();

        std::string body;
        httplib::MultipartFormDataItems items;

        httplib::MultipartFormData json_item;
        json_item.name = "payload_json";
        json_item.content = json.dump();
        json_item.content_type = "application/json";
        items.push_back(std::move(json_item));

        httplib::MultipartFormData log_item;
        log_item.name = "screenshot";
        log_item.filename = "citra-valentin-qt.log";
        log_item.content = content;
        log_item.content_type = "text/plain";
        items.push_back(std::move(log_item));

        for (const httplib::MultipartFormData& item : items) {
            body += "--" + boundary + "\r\n";
            body += "Content-Disposition: form-data; name=\"" + item.name + "\"";
            if (!item.filename.empty()) {
                body += "; filename=\"" + item.filename + "\"";
            }
            body += "\r\n";
            if (!item.content_type.empty()) {
                body += "Content-Type: " + item.content_type + "\r\n";
            }
            body += "\r\n";
            body += item.content + "\r\n";
        }

        body += "--" + boundary + "--\r\n";

        httplib::SSLClient discord_client("discordapp.com");
        std::shared_ptr<httplib::Response> webhook_response = discord_client.Post(
            "/api/webhooks/654099030720249867/"
            "FU2z5Oknxh09SnKJLl18qKxZUi0S7bNLRrQS7w975A9ppeF-L6gTPTYLqAB3huegPkVm",
            body, ("multipart/form-data; boundary=" + boundary).c_str());
        if (webhook_response == nullptr) {
            LOG_ERROR(Frontend, "Webhook request failed");
        }
        if (webhook_response != nullptr && webhook_response->status != 200) {
            LOG_ERROR(Frontend, "Webhook request failed, status code: {}, body: {}",
                      webhook_response->status, webhook_response->body);
        }
    });
}

} // namespace Log
