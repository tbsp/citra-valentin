// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QString>
#include <httplib.h>
#include <json.hpp>
#include "citra_qt/debugger/discord_logger.h"
#include "common/logging/text_formatter.h"
#include "core/settings.h"

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
    nlohmann::json json;

    if (!Settings::values.citra_username.empty()) {
        httplib::SSLClient forum_client("community.citra-emu.org");
        std::shared_ptr<httplib::Response> forum_summary_response =
            forum_client.Get(fmt::format("https://community.citra-emu.org/u/{}/summary.json",
                                         Settings::values.citra_username)
                                 .c_str());
        if (forum_summary_response == nullptr) {
            LOG_ERROR(Frontend, "Forum summary request failed");

            return;
        }
        if (forum_summary_response->status != 200) {
            LOG_ERROR(Frontend, "Forum summary request failed, status code: {}, body: {}",
                      forum_summary_response->status, forum_summary_response->body);

            return;
        }

        const nlohmann::json forum_summary = nlohmann::json::parse(forum_summary_response->body);

        if (forum_summary.count("users")) {
            const nlohmann::json user = forum_summary["users"][0];
            const std::string avatar_template = user["avatar_template"].get<std::string>();

            json["username"] = user["username"].get<std::string>();

            json["avatar_url"] =
                QString::fromStdString(std::string("https://community.citra-emu.org") +
                                       avatar_template)
                    .replace(QStringLiteral("{size}"), QStringLiteral("128"))
                    .toStdString();
        }
    }

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
    std::shared_ptr<httplib::Response> discord_response =
        discord_client.Post("/api/webhooks/654099030720249867/"
                            "FU2z5Oknxh09SnKJLl18qKxZUi0S7bNLRrQS7w975A9ppeF-L6gTPTYLqAB3huegPkVm",
                            body, ("multipart/form-data; boundary=" + boundary).c_str());
    if (discord_response == nullptr) {
        LOG_ERROR(Frontend, "Webhook request failed");
    }
    if (discord_response != nullptr && discord_response->status != 200) {
        LOG_ERROR(Frontend, "Webhook request failed, status code: {}, body: {}",
                  discord_response->status, discord_response->body);
    }
}

} // namespace Log
