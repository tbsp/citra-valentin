// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <memory>
#include <utility>
#include <json.hpp>

namespace httplib {
struct Response;
} // namespace httplib

namespace DiscordUtil {

using BaseJson = std::pair<std::shared_ptr<httplib::Response>, nlohmann::json>;

BaseJson GetBaseJson();

} // namespace DiscordUtil
