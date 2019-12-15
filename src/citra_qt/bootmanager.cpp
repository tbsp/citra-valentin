// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QApplication>
#include <QDragEnterEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLWindow>
#include <QScreen>
#include <QWindow>
#include <fmt/format.h>
#include "citra_qt/bootmanager.h"
#include "citra_qt/main.h"
#include "citra_qt/uisettings.h"
#include "core/3ds.h"
#include "core/core.h"
#include "core/frontend/scope_acquire_context.h"
#include "core/settings.h"
#include "input_common/keyboard.h"
#include "input_common/main.h"
#include "input_common/motion_emu.h"
#include "network/network.h"
#include "video_core/renderer_base.h"
#include "video_core/video_core.h"

EmuThread::EmuThread(Frontend::GraphicsContext& core_context) : core_context(core_context) {
    UpdateQtButtons();
}

EmuThread::~EmuThread() = default;

static GMainWindow* GetMainWindow() {
    for (QWidget* w : qApp->topLevelWidgets()) {
        if (GMainWindow* main = qobject_cast<GMainWindow*>(w)) {
            return main;
        }
    }
    return nullptr;
}

void EmuThread::run() {
    Frontend::ScopeAcquireContext scope(core_context);

    Core::System::GetInstance().Renderer().Rasterizer()->LoadDiskResources(
        stop_run, [this](VideoCore::LoadCallbackStage stage, std::size_t value, std::size_t total) {
            LOG_DEBUG(Frontend, "Loading stage {} progress {} {}", static_cast<u32>(stage), value,
                      total);
            emit DiskShaderCacheLoadingProgress(stage, value, total);
        });

    // Holds whether the CPU was running during the last iteration,
    // so that the DebugModeLeft signal can be emitted before the
    // next execution step.
    bool was_active = false;

    while (!stop_run) {
        if (running) {
            if (!was_active) {
                emit DebugModeLeft();
            }

            if (capture_screenshot_then_send_to_discord_server_button->GetStatus()) {
                emit CaptureScreenshotThenSendToDiscordServerRequested();
            } else if (increase_volume_button->GetStatus() && Settings::values.volume < 1.0f) {
                Settings::values.volume += 0.01f;
            } else if (decrease_volume_button->GetStatus() && Settings::values.volume >= 0.01f) {
                Settings::values.volume -= 0.01f;
            }

            Core::System::ResultStatus result = Core::System::GetInstance().RunLoop();
            if (result == Core::System::ResultStatus::ShutdownRequested) {
                // Notify frontend we shutdown
                emit ErrorThrown(result, "");
                // End emulation execution
                break;
            }
            if (result != Core::System::ResultStatus::Success) {
                this->SetRunning(false);
                emit ErrorThrown(result, Core::System::GetInstance().GetStatusDetails());
            }

            was_active = running || exec_step;
            if (!was_active && !stop_run) {
                emit DebugModeEntered();
            }
        } else if (exec_step) {
            if (!was_active) {
                emit DebugModeLeft();
            }

            exec_step = false;
            Core::System::GetInstance().SingleStep();
            emit DebugModeEntered();
            yieldCurrentThread();

            was_active = false;
        } else {
            std::unique_lock<std::mutex> lock(running_mutex);
            running_cv.wait(lock, [&] { return IsRunning() || exec_step || stop_run; });
        }
    }

    // Shutdown the core emulation
    Core::System::GetInstance().Shutdown();
}

void EmuThread::UpdateQtButtons() {
    capture_screenshot_then_send_to_discord_server_button =
        Input::CreateDevice<Input::ButtonDevice>(
            UISettings::values.capture_screenshot_then_send_to_discord_server_button.toStdString());
    increase_volume_button = Input::CreateDevice<Input::ButtonDevice>(
        UISettings::values.increase_volume_button.toStdString());
    decrease_volume_button = Input::CreateDevice<Input::ButtonDevice>(
        UISettings::values.decrease_volume_button.toStdString());
}

OpenGLWindow::OpenGLWindow(QWindow* parent, QWidget* event_handler, QOpenGLContext* shared_context)
    : QWindow(parent), event_handler(event_handler),
      context(new QOpenGLContext(shared_context->parent())) {

    // disable vsync for any shared contexts
    auto format = shared_context->format();
    format.setSwapInterval(Settings::values.use_vsync_new ? 1 : 0);
    this->setFormat(format);

    context->setShareContext(shared_context);
    context->setScreen(this->screen());
    context->setFormat(format);
    context->create();

    LOG_WARNING(Frontend, "OpenGLWindow context format Interval {}",
                context->format().swapInterval());

    LOG_WARNING(Frontend, "OpenGLWindow surface format interval {}", this->format().swapInterval());

    setSurfaceType(QWindow::OpenGLSurface);

    // TODO: One of these flags might be interesting: WA_OpaquePaintEvent, WA_NoBackground,
    // WA_DontShowOnScreen, WA_DeleteOnClose
}

OpenGLWindow::~OpenGLWindow() {
    context->doneCurrent();
}

void OpenGLWindow::Present() {
    if (!isExposed()) {
        return;
    }
    context->makeCurrent(this);
    VideoCore::g_renderer->TryPresent(100);
    context->swapBuffers(this);
    QOpenGLFunctions_3_3_Core* f = context->versionFunctions<QOpenGLFunctions_3_3_Core>();
    f->glFinish();
    QWindow::requestUpdate();
    emit Presented();
}

bool OpenGLWindow::event(QEvent* event) {
    switch (event->type()) {
    case QEvent::UpdateRequest:
        Present();
        return true;
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseMove:
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    case QEvent::FocusIn:
    case QEvent::FocusOut:
    case QEvent::FocusAboutToChange:
    case QEvent::Enter:
    case QEvent::Leave:
    case QEvent::Wheel:
    case QEvent::TabletMove:
    case QEvent::TabletPress:
    case QEvent::TabletRelease:
    case QEvent::TabletEnterProximity:
    case QEvent::TabletLeaveProximity:
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
    case QEvent::InputMethodQuery:
    case QEvent::TouchCancel:
        return QCoreApplication::sendEvent(event_handler, event);
    case QEvent::Drop:
        GetMainWindow()->DropAction(static_cast<QDropEvent*>(event));
        return true;
    case QEvent::DragResponse:
    case QEvent::DragEnter:
    case QEvent::DragLeave:
    case QEvent::DragMove:
        GetMainWindow()->AcceptDropEvent(static_cast<QDropEvent*>(event));
        return true;
    default:
        return QWindow::event(event);
    }
}

void OpenGLWindow::exposeEvent(QExposeEvent* event) {
    QWindow::requestUpdate();
    QWindow::exposeEvent(event);
}

GRenderWindow::GRenderWindow(QWidget* parent, EmuThread* emu_thread)
    : QWidget(parent), emu_thread(emu_thread) {

    setWindowTitle("Citra");
    setAttribute(Qt::WA_AcceptTouchEvents);
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setMargin(0);
    setLayout(layout);
    InputCommon::Init();
}

GRenderWindow::~GRenderWindow() {
    InputCommon::Shutdown();
}

void GRenderWindow::MakeCurrent() {
    core_context->MakeCurrent();
}

void GRenderWindow::DoneCurrent() {
    core_context->DoneCurrent();
}

void GRenderWindow::PollEvents() {}

// On Qt 5.0+, this correctly gets the size of the framebuffer (pixels).
//
// Older versions get the window size (density independent pixels),
// and hence, do not support DPI scaling ("retina" displays).
// The result will be a viewport that is smaller than the extent of the window.
void GRenderWindow::OnFramebufferSizeChanged() {
    // Screen changes potentially incur a change in screen DPI, hence we should update the
    // framebuffer size
    const qreal pixel_ratio = windowPixelRatio();
    const u32 width = this->width() * pixel_ratio;
    const u32 height = this->height() * pixel_ratio;
    UpdateCurrentFramebufferLayout(width, height);
}

void GRenderWindow::BackupGeometry() {
    geometry = QWidget::saveGeometry();
}

void GRenderWindow::RestoreGeometry() {
    // We don't want to back up the geometry here (obviously)
    QWidget::restoreGeometry(geometry);
}

void GRenderWindow::restoreGeometry(const QByteArray& geometry) {
    // Make sure users of this class don't need to deal with backing up the geometry themselves
    QWidget::restoreGeometry(geometry);
    BackupGeometry();
}

QByteArray GRenderWindow::saveGeometry() {
    // If we are a top-level widget, store the current geometry
    // otherwise, store the last backup
    if (parent() == nullptr) {
        return QWidget::saveGeometry();
    }

    return geometry;
}

qreal GRenderWindow::windowPixelRatio() const {
    return devicePixelRatio();
}

std::pair<u32, u32> GRenderWindow::ScaleTouch(const QPointF pos) const {
    const qreal pixel_ratio = windowPixelRatio();
    return {static_cast<u32>(std::max(std::round(pos.x() * pixel_ratio), qreal{0.0})),
            static_cast<u32>(std::max(std::round(pos.y() * pixel_ratio), qreal{0.0}))};
}

void GRenderWindow::closeEvent(QCloseEvent* event) {
    emit Closed();
    QWidget::closeEvent(event);
}

void GRenderWindow::keyPressEvent(QKeyEvent* event) {
    InputCommon::GetKeyboard()->PressKey(event->key());
}

void GRenderWindow::keyReleaseEvent(QKeyEvent* event) {
    InputCommon::GetKeyboard()->ReleaseKey(event->key());
}

void GRenderWindow::mousePressEvent(QMouseEvent* event) {
    if (event->source() == Qt::MouseEventSynthesizedBySystem) {
        return; // Touch input is handled in TouchBeginEvent
    }

    const QPoint position = event->pos();
    if (event->button() == Qt::LeftButton) {
        const auto [x, y] = ScaleTouch(position);
        TouchPressed(x, y);
    } else if (event->button() == Qt::MiddleButton) {
        emit MiddleClick();
    } else if (event->button() == Qt::RightButton) {
        InputCommon::GetMotionEmu()->BeginTilt(position.x(), position.y());
    }
}

void GRenderWindow::mouseMoveEvent(QMouseEvent* event) {
    if (event->source() == Qt::MouseEventSynthesizedBySystem) {
        return; // Touch input is handled in TouchUpdateEvent
    }

    const QPoint position = event->pos();
    const auto [x, y] = ScaleTouch(position);
    TouchMoved(x, y);
    InputCommon::GetMotionEmu()->Tilt(position.x(), position.y());
}

void GRenderWindow::mouseReleaseEvent(QMouseEvent* event) {
    if (event->source() == Qt::MouseEventSynthesizedBySystem)
        return; // Touch input is handled in TouchEndEvent

    if (event->button() == Qt::LeftButton)
        TouchReleased();
    else if (event->button() == Qt::RightButton)
        InputCommon::GetMotionEmu()->EndTilt();
}

void GRenderWindow::TouchBeginEvent(const QTouchEvent* event) {
    // TouchBegin always has exactly one touch point, so take the .first()
    const auto [x, y] = ScaleTouch(event->touchPoints().first().pos());
    TouchPressed(x, y);
}

void GRenderWindow::TouchUpdateEvent(const QTouchEvent* event) {
    QPointF position;
    int active_points = 0;

    // Average all active touch points
    for (const QTouchEvent::TouchPoint& point : event->touchPoints()) {
        if (point.state() &
            (Qt::TouchPointPressed | Qt::TouchPointMoved | Qt::TouchPointStationary)) {
            active_points++;
            position += point.pos();
        }
    }

    position /= active_points;

    const auto [x, y] = ScaleTouch(position);
    TouchMoved(x, y);
}

void GRenderWindow::TouchEndEvent() {
    TouchReleased();
}

bool GRenderWindow::event(QEvent* event) {
    switch (event->type()) {
    case QEvent::TouchBegin:
        TouchBeginEvent(static_cast<QTouchEvent*>(event));
        return true;
    case QEvent::TouchUpdate:
        TouchUpdateEvent(static_cast<QTouchEvent*>(event));
        return true;
    case QEvent::TouchEnd:
    case QEvent::TouchCancel:
        TouchEndEvent();
        return true;
    default:
        break;
    }

    return QWidget::event(event);
}

void GRenderWindow::focusOutEvent(QFocusEvent* event) {
    QWidget::focusOutEvent(event);
    InputCommon::GetKeyboard()->ReleaseAllKeys();
}

void GRenderWindow::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);

    OnFramebufferSizeChanged();
}

void GRenderWindow::InitRenderTarget() {
    ReleaseRenderTarget();

    GMainWindow* parent = GetMainWindow();
    QWindow* parent_win_handle = parent ? parent->windowHandle() : nullptr;
    child_window = new OpenGLWindow(parent_win_handle, this, QOpenGLContext::globalShareContext());
    child_window->create();
    child_widget = createWindowContainer(child_window, this);
    child_widget->resize(Core::kScreenTopWidth, Core::kScreenTopHeight + Core::kScreenBottomHeight);
    layout()->addWidget(child_widget);

    core_context = CreateSharedContext();
    resize(Core::kScreenTopWidth, Core::kScreenTopHeight + Core::kScreenBottomHeight);
    OnMinimalClientAreaChangeRequest(GetActiveConfig().min_client_area_size);
    BackupGeometry();
}

void GRenderWindow::ReleaseRenderTarget() {
    if (child_widget) {
        layout()->removeWidget(child_widget);
        delete child_widget;
        child_widget = nullptr;
    }
}

void GRenderWindow::OnMinimalClientAreaChangeRequest(std::pair<u32, u32> minimal_size) {
    setMinimumSize(minimal_size.first, minimal_size.second);
}

void GRenderWindow::OnEmulationStarting(EmuThread* emu_thread) {
    this->emu_thread = emu_thread;

    OnFramebufferSizeChanged();
}

void GRenderWindow::OnEmulationStopping() {
    emu_thread = nullptr;
}

void GRenderWindow::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
}

std::unique_ptr<Frontend::GraphicsContext> GRenderWindow::CreateSharedContext() const {
    return std::make_unique<GLContext>(QOpenGLContext::globalShareContext());
}

OpenGLWindow* GRenderWindow::GetOpenGLWindow() {
    return child_window;
}

GLContext::GLContext(QOpenGLContext* shared_context)
    : context(new QOpenGLContext(shared_context->parent())),
      surface(new QOffscreenSurface(nullptr)) {

    // disable vsync for any shared contexts
    auto format = shared_context->format();
    format.setSwapInterval(0);

    context->setShareContext(shared_context);
    context->setFormat(format);
    context->create();
    surface->setParent(shared_context->parent());
    surface->setFormat(format);
    surface->create();
}

void GLContext::MakeCurrent() {
    context->makeCurrent(surface);
}

void GLContext::DoneCurrent() {
    context->doneCurrent();
}
