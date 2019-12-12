// Copyright 2017 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <vector>
#include "core/core.h"
#include "core/hle/kernel/event.h"
#include "core/hle/kernel/shared_memory.h"
#include "core/hle/service/gsp/gsp.h"

namespace Service::GSP {

static std::weak_ptr<GSP_GPU> gsp_gpu;

void SignalInterrupt(InterruptId interrupt_id) {
    std::shared_ptr<Service::GSP::GSP_GPU> gpu = gsp_gpu.lock();
    ASSERT(gpu != nullptr);
    return gpu->SignalInterrupt(interrupt_id);
}

void InstallInterfaces(Core::System& system) {
    Service::SM::ServiceManager& service_manager = system.ServiceManager();
    std::shared_ptr<Service::GSP::GSP_GPU> gpu = std::make_shared<GSP_GPU>(system);
    gpu->InstallAsService(service_manager);
    gsp_gpu = gpu;

    std::make_shared<GSP_LCD>()->InstallAsService(service_manager);
}

} // namespace Service::GSP
