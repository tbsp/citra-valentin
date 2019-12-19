// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <memory>

namespace httplib {
struct Response;
} // namespace httplib

namespace DiscordUtil {

std::shared_ptr<httplib::Response> GetToken();

} // namespace DiscordUtil
