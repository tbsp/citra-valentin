// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <ostream>
#include <semver.hpp>
#include "common/common_types.h"

namespace Version {
// Citra Valentin version
extern const semver::version citra_valentin;

extern const u32 network;
extern const u8 movie;
extern const u16 shader_cache;
} // namespace Version
