// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <memory>
#include <string>
#include "common/timer.h"

namespace Common {

class Profiler {
public:
    class Scope {
    public:
        Scope(std::shared_ptr<Common::Profiler> profiler_, std::string category_,
              std::string scope_)
            : profiler(profiler_), category(std::move(category_)), scope(std::move(scope_)) {
            if (profiler == nullptr) {
                return;
            }

            timer.Start();
        }

        ~Scope() {
            if (profiler == nullptr) {
                return;
            }

            profiler->Set(category, scope, timer.GetTimeElapsed().count());
        }

    private:
        std::shared_ptr<Common::Profiler> profiler;
        Common::Timer timer;
        const std::string category;
        const std::string scope;
    };

private:
    friend class Scope;

    virtual void Set(const std::string category, const std::string scope,
                     const long long milliseconds) = 0;
};

} // namespace Common
