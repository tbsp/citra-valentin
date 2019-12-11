// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <string>
#include "common/logging/backend.h"

namespace Log {

/**
 * Backend that saves the logs in memory then sends them to Discord
 */
class DiscordBackend : public Backend {
public:
    static const char* Name() {
        return "discord";
    }

    const char* GetName() const override {
        return Name();
    }

    void Write(const Entry& entry) override;
    void Send();

private:
    std::string content;
    bool log_too_big_in_log = false;
};

} // namespace Log
