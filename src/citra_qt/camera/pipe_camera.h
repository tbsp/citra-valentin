// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <QImage>
#include <windows.h>
#include "citra_qt/camera/qt_camera_base.h"
#include "core/frontend/camera/interface.h"

namespace Camera {

class PipeCamera final : public QtCameraInterface {
public:
    PipeCamera(const std::string& config, const Service::CAM::Flip& flip);
    ~PipeCamera();

    void StartCapture() override;
    void StopCapture() override;
    void SetFrameRate(Service::CAM::FrameRate frame_rate) override {}
    QImage QtReceiveFrame() override;
    bool IsPreviewAvailable() override;

private:
    std::string config;
    HANDLE handle;
};

class PipeCameraFactory final : public QObject, public QtCameraFactory {
    Q_OBJECT

public:
    std::unique_ptr<CameraInterface> Create(const std::string& config,
                                            const Service::CAM::Flip& flip) override;
};

} // namespace Camera
