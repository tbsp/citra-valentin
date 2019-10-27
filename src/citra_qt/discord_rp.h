// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include "common/common_types.h"

namespace Core {
class System;
} // namespace Core

class DiscordRP {
public:
    DiscordRP(Core::System& system);
    ~DiscordRP();

    void Update();

private:
    Core::System& system;
    bool discord_initialized = false;
    s64 start_timestamp;
    std::string game, room_name, version, state;
};
