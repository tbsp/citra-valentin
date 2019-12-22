// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <memory>
#include <QLabel>
#include <QMainWindow>
#include <QMutex>
#include <QTimer>
#include "citra_qt/hotkeys.h"
#include "common/announce_multiplayer_room.h"
#include "core/core.h"
#include "core/frontend/framebuffer_layout.h"
#include "core/hle/service/am/am.h"
#include "ui_main.h"
#include "video_core/rasterizer_interface.h"

#ifdef CITRA_ENABLE_DISCORD_RP
#include "citra_qt/discord_rp.h"
#endif

class AboutDialog;
class Config;
class ClickableLabel;
class EmuThread;
class GameList;
enum class GameListOpenTarget;
class GameListPlaceholder;
class GImageInfo;
class GpuCommandListWidget;
class GpuCommandStreamWidget;
class GraphicsBreakpointsWidget;
class GraphicsTracingWidget;
class GraphicsVertexShaderWidget;
class GRenderWindow;
class IpcRecorderWidget;
class LleServiceModulesWidget;
class MultiplayerState;
template <typename>
class QFutureWatcher;
class QProgressBar;
class QProgressDialog;
class RegistersWidget;
class WaitTreeWidget;
class QtButtons;

class GMainWindow : public QMainWindow {
    Q_OBJECT

    /// Max number of recently loaded items to keep track of
    static const int max_recent_files_item = 10;

public:
    void FilterBarSetChecked(bool state);
    void UpdateUITheme();

    GMainWindow();
    ~GMainWindow();

    GameList* game_list;

    bool DropAction(QDropEvent* event);
    void AcceptDropEvent(QDropEvent* event);

public slots:
    void OnAppFocusStateChanged(Qt::ApplicationState state);

signals:

    /**
     * Signal that is emitted when a new EmuThread has been created and an emulation session is
     * about to start. At this time, the core system emulation has been initialized, and all
     * emulation handles and memory should be valid.
     *
     * @param emu_thread Pointer to the newly created EmuThread (to be used by widgets that need
     * to access/change emulation state).
     */
    void EmulationStarting(EmuThread* emu_thread);

    /**
     * Signal that is emitted when emulation is about to stop. At this time, the EmuThread and core
     * system emulation handles and memory are still valid, but are about become invalid.
     */
    void EmulationStopping();

    void UpdateProgress(std::size_t written, std::size_t total);
    void CIAInstallReport(Service::AM::InstallStatus status, QString filepath);
    void CIAInstallFinished();

    // Signal that tells widgets to update icons to use the current theme
    void UpdateThemedIcons();

private:
    void InitializeWidgets();
    void InitializeDebugWidgets();
    void InitializeRecentFileMenuActions();

    void SetDefaultUIGeometry();
    void SyncMenuUISettings();
    void RestoreUIState();

    void ConnectWidgetEvents();
    void ConnectMenuEvents();

    void PreventOSSleep();
    void AllowOSSleep();

    bool LoadROM(const QString& filename);
    void BootGame(const QString& filename);
    void ShutdownGame();

    void LoadAmiibo(const QString& filename);

    /**
     * Stores the filename in the recently loaded files list.
     * The new filename is stored at the beginning of the recently loaded files list.
     * After inserting the new entry, duplicates are removed meaning that if
     * this was inserted from \a OnMenuRecentFile(), the entry will be put on top
     * and remove from its previous position.
     *
     * Finally, this function calls \a UpdateRecentFiles() to update the UI.
     *
     * @param filename the filename to store
     */
    void StoreRecentFile(const QString& filename);

    /**
     * Updates the recent files menu.
     * Menu entries are rebuilt from the configuration file.
     * If there is no entry in the menu, the menu is greyed out.
     */
    void UpdateRecentFiles();

    /**
     * If the emulation is running,
     * asks the user if he really want to close the emulator
     *
     * @return true if the user confirmed
     */
    bool ConfirmClose();
    bool ConfirmChangeGame();
    void closeEvent(QCloseEvent* event) override;

    // Callouts (one-time messages)
    void ShowDiscordServerCallout();

    // Screenshots
    Layout::FramebufferLayout CreateTopScreenLayout();
    Layout::FramebufferLayout CreateBottomScreenLayout();
    void CaptureScreenshotToFile(const Layout::FramebufferLayout& layout);
    void CaptureScreenshotToClipboard(const Layout::FramebufferLayout& layout);
    void CaptureScreenshotThenSendToDiscordServer(const Layout::FramebufferLayout& layout);

    bool ValidateMovie(const QString& path, u64 program_id = 0);
    Q_INVOKABLE void OnMoviePlaybackCompleted();
    void UpdateStatusBar();
    void UpdateWindowTitle();
    void InstallCIA(QStringList filepaths);

    Ui::MainWindow ui;

    GRenderWindow* render_window;

    GameListPlaceholder* game_list_placeholder;

    std::unique_ptr<QProgressDialog> progress_dialog;

    // Status bar elements
    QProgressBar* progress_bar = nullptr;
    QLabel* message_label = nullptr;
    QLabel* emu_speed_label = nullptr;
    QLabel* emu_frametime_label = nullptr;
    QTimer status_bar_update_timer;

    MultiplayerState* multiplayer_state = nullptr;
    std::unique_ptr<Config> config;

    // Whether emulation is currently running in Citra.
    bool emulation_running = false;
    std::unique_ptr<EmuThread> emu_thread;
    // The title of the game currently running
    QString game_title;
    // The path to the game currently running
    QString game_path;

    bool auto_paused = false;

    // Movie
    bool movie_record_on_start = false;
    QString movie_record_path;

    // Video dumping
    bool video_dumping_on_start = false;
    QString video_dumping_path;
    // Whether game shutdown is delayed due to video dumping
    bool game_shutdown_delayed = false;

    // Debugger panes
    RegistersWidget* registers_widget;
    GpuCommandStreamWidget* gpu_command_stream_widget;
    GpuCommandListWidget* gpu_command_list_widget;
    GraphicsBreakpointsWidget* graphics_breakpoints_widget;
    GraphicsVertexShaderWidget* graphics_vertex_shader_widget;
    GraphicsTracingWidget* graphics_tracing_widget;
    IpcRecorderWidget* ipc_recorder_widget;
    LleServiceModulesWidget* lle_service_modules_widget;
    WaitTreeWidget* wait_tree_widget;

    QAction* actions_recent_files[max_recent_files_item];

    // Stores default icon theme search paths for the platform
    QStringList default_theme_paths;

    HotkeyRegistry hotkey_registry;

    std::unique_ptr<QImage> screenshot_image;
    QMutex screenshot_image_mutex;

    std::unique_ptr<QtButtons> buttons;

#ifdef CITRA_ENABLE_DISCORD_RP
    DiscordRP discord_rp;
#endif

private slots:
    void OnStartGame();
    void OnPauseGame();
    void OnStopGame();
    /// Called whenever a user selects a game in the game list widget.
    void OnGameListLoadFile(QString game_path);
    void OnGameListOpenFolder(u64 program_id, GameListOpenTarget target);
    void OnGameListOpenDirectory(const QString& directory);
    void OnGameListAddDirectory();
    void OnGameListShowList(bool show);
    void OnMenuLoadFile();
    void OnMenuInstallCIA();
    void OnUpdateProgress(std::size_t written, std::size_t total);
    void OnCIAInstallReport(Service::AM::InstallStatus status, QString filepath);
    void OnCIAInstallFinished();
    void OnMenuRecentFile();
    void OnConfigure(const bool goto_web = false);
    void OnLoadAmiibo();
    void OnRemoveAmiibo();
    void OnOpenCitraFolder();
    void OnToggleFilterBar();
    void OnDisplayTitleBars(bool);
    void InitializeHotkeys();
    void ToggleFullscreen();
    void ChangeScreenLayout();
    void ToggleScreenLayout();
    void OnSwapScreens();
    void OnCheats();
    void ShowFullscreen();
    void HideFullscreen();
    void ToggleWindowMode();
    void OnCreateGraphicsSurfaceViewer();
    void OnRecordMovie();
    void OnPlayMovie(const QString& filename);
    void OnStopRecordingPlayback();
    void OnStartVideoDumping();
    void OnStopVideoDumping();
    void OnCoreError(Core::System::ResultStatus, std::string);
    void OnDiskShaderCacheLoadingProgress(VideoCore::LoadCallbackStage stage, std::size_t value,
                                          std::size_t total);
    void BootGameFromArguments();

    /// Called whenever a user selects Help->About Citra Valentin
    void OnMenuAboutCitraValentin();

protected:
    void dropEvent(QDropEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
};

Q_DECLARE_METATYPE(std::size_t);
Q_DECLARE_METATYPE(Service::AM::InstallStatus);
