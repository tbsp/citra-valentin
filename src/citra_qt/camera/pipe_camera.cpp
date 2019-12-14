// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "citra_qt/camera/pipe_camera.h"
#include "common/common_types.h"

namespace Camera {

struct ImageShape {
    u32 height;
    u32 width;
    u32 Bpp;
};

PipeCamera::PipeCamera(const std::string& config, const Service::CAM::Flip& flip)
    : QtCameraInterface(flip), config(config) {}

PipeCamera::~PipeCamera() {
    StopCapture();
}

void PipeCamera::StartCapture() {
    handle = CreateFile(TEXT(R"(\\.\pipe\CitraCamera)"), GENERIC_READ | GENERIC_WRITE, 0, NULL,
                        OPEN_EXISTING, 0, NULL);

    DWORD mode = PIPE_READMODE_MESSAGE;
    SetNamedPipeHandleState(handle, &mode, NULL, NULL);
}

void PipeCamera::StopCapture() {
    CloseHandle(handle);
}

QImage PipeCamera::QtReceiveFrame() {
    ImageShape shape;
    ReadFile(handle, &shape, sizeof(shape), NULL, NULL);

    QImage image(shape.width, shape.height, QImage::Format_RGB888);
    ReadFile(handle, image.bits(), shape.width * shape.height * shape.Bpp, NULL, NULL);

    return image;
}

bool PipeCamera::IsPreviewAvailable() {
    return true;
}

std::unique_ptr<CameraInterface> PipeCameraFactory::Create(const std::string& config,
                                                           const Service::CAM::Flip& flip) {
    return std::make_unique<PipeCamera>(config, flip);
}

} // namespace Camera
