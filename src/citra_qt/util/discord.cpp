// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QDesktopServices>
#include <QString>
#include <QUrl>
#include <httplib.h>
#include "citra_qt/util/discord.h"
#include "common/logging/log.h"

namespace DiscordUtil {

nlohmann::json GetBaseJson() {
    static nlohmann::json json;
    if (!json.empty()) {
        return json;
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

        httplib::SSLClient api_client("cv-aadb.glitch.me");
        std::shared_ptr<httplib::Response> user_response =
            api_client.Post("/discord/user", std::move(api_input_json).dump(), "application/json");
        if (user_response == nullptr) {
            LOG_ERROR(Frontend, "Discord user information request failed");
            res.status = 500;
        } else if (user_response->status != 200) {
            LOG_ERROR(Frontend,
                      "Discord user information request failed, status code: {}, body: {}",
                      user_response->status, user_response->body);
            res.status = user_response->status;
            res.set_content(user_response->body,
                            httplib::detail::get_header_value(user_response->headers,
                                                              "content-type", 0, "text/plain"));
        } else {
            res.status = user_response->status;
            res.set_content(user_response->body,
                            httplib::detail::get_header_value(user_response->headers,
                                                              "content-type", 0, "text/plain"));

            const nlohmann::json discord_user_json = nlohmann::json::parse(user_response->body);
            json["username"] = discord_user_json["username"].get<std::string>();
            json["avatar_url"] = discord_user_json["avatar_url"].get<std::string>();
        }

        server.stop();
    });

    server.listen("127.0.0.1", 6310);

    return json;
}

} // namespace DiscordUtil
