set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/CMakeModules)
if (POLICY CMP0071)
    cmake_policy(SET CMP0071 NEW)
endif()

add_executable(citra-qt
    aboutdialog.cpp
    aboutdialog.h
    aboutdialog.ui
    applets/mii_selector.cpp
    applets/mii_selector.h
    applets/swkbd.cpp
    applets/swkbd.h
    bootmanager.cpp
    bootmanager.h
    camera/camera_util.cpp
    camera/camera_util.h
    camera/still_image_camera.cpp
    camera/still_image_camera.h
    camera/qt_camera_base.cpp
    camera/qt_camera_base.h
    camera/qt_multimedia_camera.cpp
    camera/qt_multimedia_camera.h
    cheats.cpp
    cheats.h
    cheats.ui
    citra-qt.rc
    configuration/config.cpp
    configuration/groups/config_controls.cpp
    configuration/groups/config_core.cpp
    configuration/groups/config_renderer.cpp
    configuration/groups/config_layout.cpp
    configuration/groups/config_utility.cpp
    configuration/groups/config_audio.cpp
    configuration/groups/config_camera.cpp
    configuration/groups/config_data_storage.cpp
    configuration/groups/config_system.cpp
    configuration/groups/config_miscellaneous.cpp
    configuration/groups/config_debugging.cpp
    configuration/groups/config_lle.cpp
    configuration/groups/config_web_service.cpp
    configuration/groups/config_ui.cpp
    configuration/groups/config_ui_layout.cpp
    configuration/groups/config_game_list.cpp
    configuration/groups/config_paths.cpp
    configuration/groups/config_shortcuts.cpp
    configuration/groups/config_multiplayer.cpp
    configuration/groups/config_hacks.cpp
    configuration/config.h
    configuration/configure.ui
    configuration/configure_audio.cpp
    configuration/configure_audio.h
    configuration/configure_audio.ui
    configuration/configure_camera.cpp
    configuration/configure_camera.h
    configuration/configure_camera.ui
    configuration/configure_debug.cpp
    configuration/configure_debug.h
    configuration/configure_enhancements.cpp
    configuration/configure_enhancements.h
    configuration/configure_custom_layout.cpp
    configuration/configure_custom_layout.h
    configuration/configure_debug.ui
    configuration/configure_dialog.cpp
    configuration/configure_dialog.h
    configuration/configure_general.cpp
    configuration/configure_general.h
    configuration/configure_general.ui
    configuration/configure_graphics.cpp
    configuration/configure_graphics.h
    configuration/configure_graphics.ui
    configuration/configure_hotkeys.cpp
    configuration/configure_hotkeys.h
    configuration/configure_hotkeys.ui
    configuration/configure_input.cpp
    configuration/configure_input.h
    configuration/configure_input.ui
    configuration/configure_motion_touch.cpp
    configuration/configure_motion_touch.h
    configuration/configure_motion_touch.ui
    configuration/configure_system.cpp
    configuration/configure_system.h
    configuration/configure_system.ui
    configuration/configure_ui.cpp
    configuration/configure_ui.h
    configuration/configure_ui.ui
    configuration/configure_releases.cpp
    configuration/configure_releases.h
    configuration/configure_releases.ui
    configuration/configure_web.cpp
    configuration/configure_web.h
    configuration/configure_web.ui
    debugger/console.h
    debugger/console.cpp
    debugger/graphics/graphics.cpp
    debugger/graphics/graphics.h
    debugger/graphics/graphics_breakpoint_observer.cpp
    debugger/graphics/graphics_breakpoint_observer.h
    debugger/graphics/graphics_breakpoints.cpp
    debugger/graphics/graphics_breakpoints.h
    debugger/graphics/graphics_breakpoints_p.h
    debugger/graphics/graphics_cmdlists.cpp
    debugger/graphics/graphics_cmdlists.h
    debugger/graphics/graphics_surface.cpp
    debugger/graphics/graphics_surface.h
    debugger/graphics/graphics_tracing.cpp
    debugger/graphics/graphics_tracing.h
    debugger/graphics/graphics_vertex_shader.cpp
    debugger/graphics/graphics_vertex_shader.h
    debugger/ipc/record_dialog.cpp
    debugger/ipc/record_dialog.h
    debugger/ipc/record_dialog.ui
    debugger/ipc/recorder.cpp
    debugger/ipc/recorder.h
    debugger/ipc/recorder.ui
    debugger/lle_service_modules.cpp
    debugger/lle_service_modules.h
    debugger/profiler.cpp
    debugger/profiler.h
    debugger/profiler.ui
    debugger/registers.cpp
    debugger/registers.h
    debugger/registers.ui
    debugger/wait_tree.cpp
    debugger/wait_tree.h
    game_list.cpp
    game_list.h
    game_list_p.h
    game_list_worker.cpp
    game_list_worker.h
    hotkeys.cpp
    hotkeys.h
    main.cpp
    main.h
    main.ui
    multiplayer/chat_room.cpp
    multiplayer/chat_room.h
    multiplayer/chat_room.ui
    multiplayer/client_room.h
    multiplayer/client_room.cpp
    multiplayer/client_room.ui
    multiplayer/direct_connect.cpp
    multiplayer/direct_connect.h
    multiplayer/direct_connect.ui
    multiplayer/host_room.cpp
    multiplayer/host_room.h
    multiplayer/host_room.ui
    multiplayer/lobby.cpp
    multiplayer/lobby.h
    multiplayer/lobby.ui
    multiplayer/lobby_p.h
    multiplayer/message.cpp
    multiplayer/message.h
    multiplayer/moderation_dialog.cpp
    multiplayer/moderation_dialog.h
    multiplayer/moderation_dialog.ui
    multiplayer/state.cpp
    multiplayer/state.h
    multiplayer/validation.h
    qt_image_interface.cpp
    qt_image_interface.h
    uisettings.cpp
    uisettings.h
    util/clickable_label.cpp
    util/clickable_label.h
    util/sequence_dialog/sequence_dialog.cpp
    util/sequence_dialog/sequence_dialog.h
    util/spinbox.cpp
    util/spinbox.h
    util/util.cpp
    util/util.h
    $<$<BOOL:${WIN32}>:
        camera/dshow_camera.cpp
        camera/dshow_camera.h
    >
    $<$<BOOL:${CITRA_ENABLE_DISCORD_RP}>:
        discord_rp.cpp
        discord_rp.h
    >
)

file(GLOB_RECURSE ICONS ${PROJECT_SOURCE_DIR}/dist/icons/*)
file(GLOB_RECURSE THEMES ${PROJECT_SOURCE_DIR}/dist/qt_themes/*)

target_sources(citra-qt PRIVATE ${ICONS} ${THEMES})

if(CITRA_ENABLE_DISCORD_RP)
    target_sources(citra-qt PRIVATE
        discord_rp.cpp
        discord_rp.h
    )

    target_link_libraries(citra-qt PRIVATE discord-rpc)
    target_compile_definitions(citra-qt PRIVATE -DCITRA_ENABLE_DISCORD_RP)
endif()

# So Discord doesn't say it's a official build
set_target_properties(citra-qt PROPERTIES OUTPUT_NAME "citra-valentin-qt")

if(WIN32)
    # Compile as a Win32 GUI application instead of a console application
    target_link_libraries(citra-qt PRIVATE libdshowcapture Qt5::WinMain)
    set_target_properties(citra-qt PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
endif()

create_target_directory_groups(citra-qt)

get_directory_property(OPENSSL_LIBS
        DIRECTORY ${PROJECT_SOURCE_DIR}/externals/libressl
        DEFINITION OPENSSL_LIBS)
target_compile_options(citra-qt PRIVATE -DCPPHTTPLIB_OPENSSL_SUPPORT)

target_link_libraries(citra-qt PRIVATE audio_core common core input_common network video_core)
target_link_libraries(citra-qt PRIVATE Boost::boost glad json-headers nihstro-headers ${OPENSSL_LIBS} httplib Qt5::OpenGL Qt5::Widgets Qt5::Multimedia)
target_link_libraries(citra-qt PRIVATE ${PLATFORM_LIBRARIES} Threads::Threads)

target_compile_definitions(citra-qt PRIVATE
    # Use QStringBuilder for string concatenation to reduce
    # the overall number of temporary strings created.
    -DQT_USE_QSTRINGBUILDER

    # Disable implicit type narrowing in signal/slot connect() calls.
    -DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT

    # Disable unsafe overloads of QProcess' start() function.
    -DQT_NO_PROCESS_COMBINED_ARGUMENT_START

    # Disable implicit QString->QUrl conversions to enforce use of proper resolving functions.
    -DQT_NO_URL_CAST_FROM_STRING
)

if (MSVC)
    include(CopyCitraQt5Deps)
    include(CopyCitraSDLDeps)
    copy_citra_Qt5_deps(citra-qt)
    copy_citra_SDL_deps(citra-qt)

    if (ENABLE_FFMPEG)
        include(CopyCitraFFmpegDeps)
        copy_citra_FFmpeg_deps(citra-qt)
    endif()
endif()
