// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "citra_qt/camera/dshow_camera.h"
#include "common/common_types.h"
#include "common/string_util.h"

namespace Camera {

DirectShowCamera::DirectShowCamera(const std::string& config, const Service::CAM::Flip& flip)
    : QtCameraInterface(flip) {
    if (!device.ResetGraph()) {
        LOG_ERROR(Frontend, "ResetGraph failed: {}", GetLastErrorMsg());
        return;
    }

    DShow::VideoConfig video_config;
    if (config.empty()) {
        std::vector<DShow::VideoDevice> devices;
        DShow::Device::EnumVideoDevices(devices);
        video_config.path = devices[0].path;
    } else {
        video_config.path = Common::UTF8ToUTF16W(config);
    }
    video_config.callback = [this](const DShow::VideoConfig& video_config, unsigned char* data,
                                   size_t size, long long startTime, long long stopTime) {
        switch (video_config.format) {
        case DShow::VideoFormat::XRGB:
            image = QImage(data, video_config.cx, video_config.cy, QImage::Format_ARGB32)
                        .convertToFormat(QImage::Format_RGB888);
            break;
        default:
            LOG_ERROR(Service_CAM, "Format not supported: {}",
                      static_cast<int>(video_config.format));
            break;
        }
    };

    if (!device.SetVideoConfig(&video_config)) {
        LOG_ERROR(Service_CAM, "SetVideoConfig: {}", GetLastErrorMsg());
        return;
    }

    if (!device.ConnectFilters()) {
        LOG_ERROR(Service_CAM, "ConnectFilters: {}", GetLastErrorMsg());
    }
}

DirectShowCamera::~DirectShowCamera() {
    StopCapture();
}

void DirectShowCamera::StartCapture() {
    switch (device.Start()) {
    case DShow::Result::Error:
        LOG_ERROR(Service_CAM, "Start failed: {}", GetLastErrorMsg());
        break;
    case DShow::Result::InUse:
        LOG_ERROR(Service_CAM, "Camera in use!");
        break;
    case DShow::Result::Success:
        break;
    }
}

void DirectShowCamera::StopCapture() {
    device.Stop();
    device.ResetGraph();
}

QImage DirectShowCamera::QtReceiveFrame() {
    return image;
}

bool DirectShowCamera::IsPreviewAvailable() {
    return device.Valid();
}

std::unique_ptr<CameraInterface> DirectShowCameraFactory::Create(const std::string& config,
                                                                 const Service::CAM::Flip& flip) {
    return std::make_unique<DirectShowCamera>(config, flip);
}

} // namespace Camera
