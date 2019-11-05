// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <QImage>
#include <dshowcapture.hpp>
#include "citra_qt/camera/qt_camera_base.h"
#include "core/frontend/camera/interface.h"

namespace Camera {

class DirectShowCamera final : public QtCameraInterface {
public:
    DirectShowCamera(const std::string& config, const Service::CAM::Flip& flip);
    ~DirectShowCamera();

    void StartCapture() override;
    void StopCapture() override;
    void SetFrameRate(Service::CAM::FrameRate frame_rate) override {}
    QImage QtReceiveFrame() override;
    bool IsPreviewAvailable() override;

private:
    DShow::Device device;
    QImage image;
};

class DirectShowCameraFactory final : public QObject, public QtCameraFactory {
    Q_OBJECT

public:
    std::unique_ptr<CameraInterface> Create(const std::string& config,
                                            const Service::CAM::Flip& flip) override;
};

} // namespace Camera
