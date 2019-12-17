// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QDesktopServices>
#include <QString>
#include <QUrl>
#include <httplib.h>
#include "citra_qt/uisettings.h"
#include "citra_qt/util/discord.h"
#include "common/logging/log.h"

namespace DiscordUtil {

BaseJson GetBaseJson() {
    static BaseJson base_json;
    if (base_json.first != nullptr && base_json.first->status == 200) {
        return base_json;
    } else {
        base_json.second.clear();
    }

    QDesktopServices::openUrl(
        QUrl(QStringLiteral("https://discordapp.com/api/oauth2/"
                            "authorize?client_id=633487273413050418&redirect_uri=http%3A%2F%"
                            "2F127.0.0.1%3A6310&response_type=code&scope=identify")));

    httplib::Server server;

    server.Get("/", [&](const httplib::Request& req, httplib::Response& res) {
        const std::string code = req.params.find("code")->second;

        nlohmann::json api_input_json;
        api_input_json["code"] = code;

        httplib::SSLClient api_client(
            QUrl(UISettings::values.cv_web_api_url).host().toStdString().c_str());
        base_json.first =
            api_client.Post("/discord/user", std::move(api_input_json).dump(), "application/json");
        if (base_json.first == nullptr) {
            LOG_ERROR(Frontend, "Discord user information request failed");
            res.status = 500;
        } else if (base_json.first->status != 200) {
            LOG_ERROR(Frontend,
                      "Discord user information request failed, status code: {}, body: {}",
                      base_json.first->status, base_json.first->body);
            res.status = base_json.first->status;
            res.set_content(base_json.first->body,
                            httplib::detail::get_header_value(base_json.first->headers,
                                                              "content-type", 0, "text/plain"));
        } else {
            res.status = base_json.first->status;
            res.set_content(base_json.first->body,
                            httplib::detail::get_header_value(base_json.first->headers,
                                                              "content-type", 0, "text/plain"));

            if (std::strstr(httplib::detail::get_header_value(base_json.first->headers,
                                                              "content-type", 0, "text/plain"),
                            "application/json") != nullptr) {
                const nlohmann::json discord_user_json =
                    nlohmann::json::parse(base_json.first->body);
                base_json.second["username"] = discord_user_json["username"].get<std::string>();
                base_json.second["avatar_url"] = discord_user_json["avatar_url"].get<std::string>();
            }
        }

        server.stop();
    });

    server.listen("127.0.0.1", 6310);

    return base_json;
}

} // namespace DiscordUtil
