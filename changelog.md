# (Unreleased)

## Fixes

- Qt: Don't send game name if it's empty

## Minor Changes

- Remove `multiply_input` because it didn't work
- Lots of code style changes

# 8.1.0

## Features

- SDL Analogs: `multiply_input` parameter in the ini file. Range: 1.0-2.0.
- Qt: Key/controller buttons for changing the volume
- Qt/GameList: Configuration file for each game (Start Using Game Settings right click context menu option)

# 8.0.2

## Fixes

- Qt: CaptureScreenshotThenSendToDiscordServer: Send Citra account username if the account was created very recently
- Qt: CaptureScreenshotThenSendToDiscordServer: Handle empty game name
- Qt/ConfigureWeb: Fix missing 'n' in Hardware Shader - Accurate Multiplication
- Qt/DiscordLogger: Send Citra account username if the account was created very recently

## Minor Changes

- Settings: Add Valentin to Citra Configuration log
- Qt: Add tooltip to Discord Logger and Capture Screenshot Then Send To Discord Server
- Qt/ConfigureWeb: Add more usages of the Citra account
- Update externals

# 8.0.1

## Fixes

- Qt: Crashes in Discord Logger and after pressing the Capture Screenshot Then Send To Discord Server key/button when using a Citra account created very recently.
- Qt/GameList: Configuration not saved if CV crashes

# 8.0.0

## Breaking Changes

- OpenGL disk shader cache: Add -cv suffix to transferable and precompiled folders.

# 7.1.0

## Features

- Disk shader cache re-added
- Qt: Custom profiler. MicroProfile was removed.
- Qt: Discord logger

# 7.0.0

## Breaking Changes

- Disk shader cache was removed because it causes very high memory usage in LM2/LMDM.
- vvanelslande-shaders folder renamed to shaders

# 6.0.0

## Breaking Changes

- Qt: Change Capture Screenshot Then Send To Discord Server hotkey to a button, now that feature can be used with a gamepad.

## Fixes

- Qt: Capture Screenshot Then Send To Discord Server: Always include game name

# 5.0.1

## Fixes

- Qt: Screenshot related crashes

## Minor Changes

- Update externals

# 5.0.0

## Breaking Changes

- Qt: Rename capture screenshot hotkey

## Features

- Qt: Options and hotkeys to capture screenshot then copy it to clipboard and capture screenshot then send it to #screenshots-from-cv in the [Discord server](https://discord.gg/fPmDUaY)

## Fixes

- Qt: Make configuration window smaller

## Minor Changes

- Update externals

## Qt: New Capture Screenshot Hotkey Defaults

- Capture Screenshot Then Copy To Clipboard: Ctrl+B
- Capture Screenshot Then Send To Discord Server: Ctrl+I

# 4.7.0

## Features

- Qt: Windows-only updater (CVU) release list
- Qt: Disk shader cache loading progress bar

## Fixes

- DSP HLE: Luigi's Mansion 2 / Dark Moon freeze, ZeusIota wrote the fix but the fix broke Super Mario 3D Land's audio. I fixed that by changing `MAX_SAMPLES` to `791408`. If the audio loops infinitely, create a issue and include the log file (`citra-valentin.log` for the CLI or `citra-valentin-qt.log` for the GUI).

## Minor Changes

- Qt: Add Discord server invite callout
- Qt: Rename Versions tab to Releases

# 4.6.0

## Features

- Qt/ConfigureVersions/Recent Releases: Change double click to right click and add Open Changelog and options to download builds for Linux and Windows

# 4.5.0

## Features

### Qt Versions tab

- Versions Installed Using [CVU](https://github.com/vvanelslande/cvu): right click a version to start it, open the changelog in your browser, open it's location or delete it (if it's not the version you're using).
- Recent Releases: double click a version to open the release in your browser.

## Minor Changes

- Qt: rename Open EXE Location to Open Executable Location, use a cross platform function and show the option on Linux.
- Common: LinearDiskCache removed, it wasn't used.
- Externals: Update fmt, [commits](https://github.com/fmtlib/fmt/compare/d22e4ad85b477b4f2b4ffb034d1212cc9afcfe49...1219b65f2167fabe58ce77f4bd7447da5fa02611)

# 4.4.1

## Fixes

- Discord server invite updated
- Qt/ConfigureWeb/Telemetry: DSP LLE not saved

# 4.4.0

## Features

- Qt: Discord RP settings to choose what is sent
- Qt: Telemetry, you choose what is sent and where it's sent.

## Fixes

- Qt/ConfigureGraphics: Make Sharper Distant Objects unchangeable while emulation is running

## Minor Changes

- CMakeLists: Remove `if (ENABLE_QT_TRANSLATION)`
- Externals: Update fmt, zstd and cubeb

# 4.3.1

## Fixes

- [LLE audio required, otherwise your PC will freeze] Luigi's Mansion 2 / Dark Moon ghost capture crash

# 4.3.0

## Features

### Qt: Custom Screen Refresh Rate Hotkeys (requested by `OHU#3151` in the [HunsterVerse Discord server](https://hunstermonter.net))

- Toggle Custom Screen Refresh Rate (default: Ctrl+R)
- Change Custom Screen Refresh Rate (default: Ctrl+Alt+R)

## Minor Changes

- Externals: Bump [fmt](https://github.com/fmtlib/fmt) from [5981588](https://github.com/fmtlib/fmt/commit/5981588565191fe7c8934021f711286f8ea61d93) to [a64f60c8](https://github.com/fmtlib/fmt/commit/a64f60c8499dcca96868e9beda4c76915baae29a)
- [Externals: Bump `zstd` from `d3ec368` to `e8faa09` (Citra Valentin PR 71)](https://github.com/vvanelslande/citra/pull/71)
- [Externals: Bump `teakra` from `549796e` to `bcb0ade` (Citra Valentin PR 72)](https://github.com/vvanelslande/citra/pull/72)

# 4.2.0

## Features

- [Presentation microprofile scopes](https://github.com/citra-emu/citra/pull/4940/commits/65613cce819cdcc0ccbf9ea19210abaffdaf2b1e)

## Minor Changes

- OpenGL disk shader cache: change entry count log messages
- [Qt: Remove high dpi scaling flag as it needs more work first](https://github.com/citra-emu/citra/pull/4940/commits/9ae3eb4a3038c6664b0be7c88b5beb1cc735cf6d)

# 4.1.0

## Features

- Qt: Toggle Custom Layout hotkey (default: Ctrl+C), requested by `adyhawk#6980` in the [HunsterVerse Discord server](https://hunstermonter.net)

## Minor Changes

- Externals: Bump `fmt` from `4ca6821` to `5981588`

# 4.0.2

## Fixes

- Azure Pipelines: a commit changelog was added to the release descriptions

# 4.0.1

## Fixes

- SDL: Crash after starting a game
- SDL: Move `Starting in fullscreen mode...` log to `EmuWindow_SDL2`, was logged if using `--help` or `--version`

## Minor Changes

- Externals: Bump `fmt` from `3bc28fc` to `4ca6821`
- SDL: Remove `is_fullscreen` hack because it's not needed
- Add `.prettierrc.json`

# 4.0.0

## Breaking Changes

- Qt: Rename `Exit Citra Valentin` hotkey to `Exit`

## Features

- Qt: Add `Toggle Hardware Shader` hotkey
- [Threaded CPU vertex shader processing (Citra PR 4270)](https://github.com/citra-emu/citra/pull/4270)

## Minor Changes

- Add this file
- Qt: Add `QStringLiteral()` to hotkey strings in `citra_qt/main.cpp`
- Qt: Add a menu option to open the changelog of this version in the Help menu

# 3.0.2

## Fixes

- [Software Keyboard: Remove text memory clearing (Citra PR 5016)](https://github.com/citra-emu/citra/pull/5016)

## Minor Changes

- [Qt/AboutDialog: Open raw license](https://github.com/vvanelslande/citra/commit/b8e33b2df0a7c660ee8e9b3d8112c4e21b0deb70)
- [Qt/ConfigureGraphics: Change Ignore Format Reinterpretation tooltip from `This breaks Paper Mario.` to `Ignores flushing surfaces from CPU memory if the surface was created by the GPU and has a different format. This can speed up many games, potentially break some, but is rightfully just a hack as a placeholder for GPU texture encoding/decoding`](https://github.com/vvanelslande/citra/commit/767f2d2ab16380c564b661f859c4767e8fa6a46a)
- [SDL/DefaultINI: Add line](https://github.com/vvanelslande/citra/commit/8892c3dd2c7a040d9ac5e08c453263e5ce787f3a)
- [SDL/DefaultINI: Change a `true` to `1`](https://github.com/vvanelslande/citra/commit/8892c3dd2c7a040d9ac5e08c453263e5ce787f3a)
- [SDL/DefaultINI: Add comment for Ignore Format Reinterpretation (`ignore_format_reinterpretation`)](https://github.com/vvanelslande/citra/commit/c3c68f38a8dd149417c8a024990bbe69666fc6ec)
- [Externals: Bump `fmt` from ef7369c to 3bc28fc (Citra Valentin PR 67)](https://github.com/vvanelslande/citra/pull/67)

# 3.0.1

## Fixes

- [Qt/GRenderWindow: The touch point is incorrect in OpenGLWindow when the render target is initialized for the first time with single window mode disabled.](https://github.com/vvanelslande/citra/commit/ec50c16c322499ea0878463184a9af0e31137e38)

## Minor Changes

- [Externals: Bump `fmt` from 1200a34 to ef7369c (Citra Valentin PR 67)](https://github.com/vvanelslande/citra/pull/65)
- [Externals: Bump `xbyak` from f32836d to 20ee4c2 (Citra Valentin PR 66)](https://github.com/vvanelslande/citra/pull/66)

# 3.0.0

## Breaking Changes

### Movie header changes

- [Magic changed from `CTM 0x1C` to `CVM 0x01`](https://github.com/vvanelslande/citra/commit/32080bdc6c49a1f6e2575d2813787f8ae6c96ae2#diff-8c34e25a6a635fe90347d4de900c9cffR112)
- [Version number removed, now it's the last magic byte (it's 0x01 in 3.0.0)](https://github.com/vvanelslande/citra/commit/32080bdc6c49a1f6e2575d2813787f8ae6c96ae2#diff-8c34e25a6a635fe90347d4de900c9cffL118)

## Features

- [Qt: Re-add drag and drop support](https://github.com/vvanelslande/citra/commit/688eec774b63c35d4006e307e3f85e34299265b2)

## Minor Changes

- Move movie, network and shader cache versions to version.cpp and version.h
- [CMake: Change `Set bundled sdl2/qt as dependent options.` to `Set bundled SDL2/Qt as dependent options.`](https://github.com/vvanelslande/citra/commit/a0339d269960f38011a7b718068732674a511730)
- [Logging: Silence no return value warnings (Citra PR 5006)](https://github.com/citra-emu/citra/pull/5006)
- [Qt/GMainWindow: Move some code from `OnStartGame` to `BootGame`](https://github.com/vvanelslande/citra/commit/5eaff938d28e9fa7f0800b3bf6fa4df324fe470b)
- [Qt/GRenderWindow: Fix window resizing bug](https://github.com/vvanelslande/citra/commit/92ea85f2608187b3105497f81d269e06eb87ec12)
- [Qt/DirectShowCamera: Add image mutex](https://github.com/vvanelslande/citra/commit/5ae3742a224810823bc1555e929f749734c4652f#diff-18132a2049b61c6b5402d47e799b4d65R31)
- [Qt/DirectShowCamera: Change format not supported `LOG_ERROR` to a `UNIMPLEMENTED_MSG`](https://github.com/vvanelslande/citra/commit/5ae3742a224810823bc1555e929f749734c4652f#diff-18132a2049b61c6b5402d47e799b4d65R37)
- [SDL: Add `std::` to some `exit(1)`](https://github.com/vvanelslande/citra/commit/32080bdc6c49a1f6e2575d2813787f8ae6c96ae2#diff-7e7befdc42c1e96e42041ecfe89e53e7L109)
- [SDL: Rename `LodePNGImageInterface` to `LodePngImageInterface`](https://github.com/vvanelslande/citra/commit/32080bdc6c49a1f6e2575d2813787f8ae6c96ae2#diff-5b9aa5c6d647ecfb18298b2de8753606L9)
- [SDL: Change some {} to =](https://github.com/vvanelslande/citra/commit/32080bdc6c49a1f6e2575d2813787f8ae6c96ae2#diff-7e7befdc42c1e96e42041ecfe89e53e7L366)
- [SDL: Add `-x`/`--fullscreen-display-index` (Default: 0) option to change fullscreen display](https://github.com/vvanelslande/citra/commit/002870dc4a6b11903d2fb630b9c3c5b53175f607)
- [Frontend/ImageInterface: Rename DecodePNG to DecodePng and EncodePNG to EncodePng](https://github.com/vvanelslande/citra/commit/32080bdc6c49a1f6e2575d2813787f8ae6c96ae2#diff-e20f8b554eb29a6dc42eef238a1875caL18)
- [VideoCore: Remove MSVC hack comment on TevStageConfigRaw (Citra PR 5004)](https://github.com/citra-emu/citra/pull/5004)
- [Externals: Bump `cubeb` from 5f68e42 to e837916 (Citra Valentin PR 64)](https://github.com/vvanelslande/citra/pull/64)
- [Externals: Bump `libdshowcapture` from 6388a01 to 9f672d8 (Citra Valentin PR 63)](https://github.com/vvanelslande/citra/pull/63)
- [Externals: Bump `zstd` from 1d60704 to d3ec368 (Citra Valentin PR 62)](https://github.com/vvanelslande/citra/pull/62)
- [Externals: Bump `fmt` from c68703c to 1200a34 (Citra Valentin PR 61)](https://github.com/vvanelslande/citra/pull/61)
- [Externals: Bump `discord-rpc` from eff23a7 to e4c0c56 (Citra Valentin PR 60)](https://github.com/vvanelslande/citra/pull/60)
- [Externals: Bump `catch` from a537cca to 9a89631 (Citra Valentin PR 52)](https://github.com/vvanelslande/citra/pull/52)
- [Externals: Bump `lodepng` from e1d431b to dc3f19b (Citra Valentin PR 48)](https://github.com/vvanelslande/citra/pull/48)

# 2.11.2

## Fixes

- [SDL: Use screen size in fullscreen (no display option)](https://github.com/vvanelslande/citra/commit/a271702448fe4cf22887ad45bd9e77b370a6c02e)

# 2.11.1

## Fixes

- [Moderation by room host in dedicated rooms (Citra PR 5002)](https://github.com/citra-emu/citra/pull/5002)

## Minor Changes

- [Qt: GameList & Multiplayer: Specify string conversions explicitly (Citra PR 4992)](https://github.com/citra-emu/citra/pull/4992)

# 2.11.0

## Features

- [Qt/Configuration: Add custom layout tab to Graphics](https://github.com/vvanelslande/citra/commit/f4cb0d3ff0da2a1fc8e8b54c4496ec340ec40a85)

## Minor Changes

- [README: Add prebuilt binary requirements](https://github.com/vvanelslande/citra/commit/4657ec8fd3081b52d3418f28083996f1ed7feff6)
- [Qt: Rename `About Citra` to `About Citra Valentin` and add Discord link](https://github.com/vvanelslande/citra/commit/23dcf78937bdb93f2582c3a767b9f659d8e40239)
- [Externals: Bump `catch` from 32733e0 to f23f968 (Citra Valentin PR 38)](https://github.com/vvanelslande/citra/pull/38)
- [Externals: Bump `fmt` from 1f91815 to 2145a7b (Citra Valentin PR 39)](https://github.com/vvanelslande/citra/pull/39)
- [Externals: Bump `zstd` from 7d85f35 to 4b1ac69 (Citra Valentin PR 40)](https://github.com/vvanelslande/citra/pull/40)
- [Externals: Bump `lodepng` from 4d4a406 to e1d431b (Citra Valentin PR 41)](https://github.com/vvanelslande/citra/pull/41)

# 2.10.5

## Minor Changes

- [Qt: Move Discord RP update from OnStopGame to ShutdownGame](https://github.com/vvanelslande/citra/commit/3b9d7c127f0c720c3d7d21883404dde81204c330)
- [Externals: Bump `zstd` from cabbb1e to 7d85f35 (Citra Valentin PR 35)](https://github.com/vvanelslande/citra/pull/35)

# 2.10.4

## Minor Changes

- [Revert some shader type cast fixes for OpenGL ES but this fork will never support Android](https://github.com/vvanelslande/citra/commit/a8301259614894fe0be37c82b2d8af20634cc730) ([Citra PR 4859 fixed the shader type casts](https://github.com/citra-emu/citra/pull/4859))

# 2.10.3

## Fixes

- [Dedicated Room: Fix crash (count != 0) when closing the room (Citra PR 4998)](https://github.com/citra-emu/citra/pull/4998)

## Minor Changes

- [Bump `zstd` from 2ed5344 to cabbb1e (Citra Valentin PR 33)](https://github.com/vvanelslande/citra/pull/33)
- [Bump `lodepng` from 015f021 to 4610888 (Citra Valentin PR 34)](https://github.com/vvanelslande/citra/pull/34)

# 2.10.2

## Fixes

- [Azure Pipelines: missing Windows tar](https://github.com/vvanelslande/citra/commit/b99103d5cf6f6b3e75aaa3e9a37da869e7218021)

# 2.10.1

## Features

- [Base64 token support (Citra PR 4993)](https://github.com/citra-emu/citra/pull/4993)

## Fixes

- [Rename input profile (Citra PR 4995)](https://github.com/citra-emu/citra/pull/4995)
