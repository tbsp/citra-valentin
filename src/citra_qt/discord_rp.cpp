// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <discord_rpc.h>
#include <fmt/format.h>
#include "citra_qt/discord_rp.h"
#include "citra_qt/uisettings.h"
#include "common/version.h"
#include "core/core.h"
#include "network/network.h"

DiscordRP::DiscordRP(Core::System& system) : system(system) {
    Update();
}

DiscordRP::~DiscordRP() {
    Update();
}

void DiscordRP::Update() {
    if (UISettings::values.enable_discord_rp && !discord_initialized) {
        start_timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                              std::chrono::system_clock::now().time_since_epoch())
                              .count();
        version = fmt::format("Citra Valentin Nintendo 3DS emulator version {}",
                              Version::citra_valentin.to_string());

        Discord_Initialize("633487273413050418", NULL, 0, NULL);
        discord_initialized = true;
    }

    if (!UISettings::values.enable_discord_rp && discord_initialized) {
        Discord_Shutdown();
        discord_initialized = false;

        return;
    }

    if (!discord_initialized) {
        return;
    }

    DiscordRichPresence presence{};
    presence.largeImageKey = "icon";
    presence.largeImageText = version.c_str();
    presence.startTimestamp = start_timestamp;

    state = system.IsPoweredOn() ? "In-game" : "Idling";

    if (UISettings::values.discord_rp_show_room_information) {
        if (std::shared_ptr<Network::RoomMember> room_member = Network::GetRoomMember().lock()) {
            if (room_member->IsConnected()) {
                room_name = room_member->GetRoomInformation().name;
                state += fmt::format(" ({} of {})", room_member->GetMemberInformation().size(),
                                     room_member->GetRoomInformation().member_slots);

                presence.smallImageKey = "connected";
                presence.smallImageText = room_name.c_str();
                presence.state = state.c_str();
            } else if (!room_name.empty()) {
                room_name.clear();
            }
        }
    }

    presence.state = state.c_str();

    if (UISettings::values.discord_rp_show_game_name && system.IsPoweredOn()) {
        system.GetAppLoader().ReadTitle(game);

        presence.details = system.IsPoweredOn() ? game.c_str() : "";
    } else if (!game.empty()) {
        game.clear();
    }

    Discord_UpdatePresence(&presence);
}
