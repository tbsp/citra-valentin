// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QSettings>
#include "citra_qt/configuration/config.h"
#include "citra_qt/uisettings.h"

// clang-format off
const std::array<UISettings::Shortcut, 37> default_hotkeys{
    {{QStringLiteral("2x Native Internal Resolution"),                  QStringLiteral("Main Window"), {QStringLiteral("Alt+2"), Qt::ApplicationShortcut}},
     {QStringLiteral("3x Native Internal Resolution"),                  QStringLiteral("Main Window"), {QStringLiteral("Alt+3"), Qt::ApplicationShortcut}},
     {QStringLiteral("4x Native Internal Resolution"),                  QStringLiteral("Main Window"), {QStringLiteral("Alt+4"), Qt::ApplicationShortcut}},
     {QStringLiteral("5x Native Internal Resolution"),                  QStringLiteral("Main Window"), {QStringLiteral("Alt+5"), Qt::ApplicationShortcut}},
     {QStringLiteral("6x Native Internal Resolution"),                  QStringLiteral("Main Window"), {QStringLiteral("Alt+6"), Qt::ApplicationShortcut}},
     {QStringLiteral("7x Native Internal Resolution"),                  QStringLiteral("Main Window"), {QStringLiteral("Alt+7"), Qt::ApplicationShortcut}},
     {QStringLiteral("8x Native Internal Resolution"),                  QStringLiteral("Main Window"), {QStringLiteral("Alt+8"), Qt::ApplicationShortcut}},
     {QStringLiteral("9x Native Internal Resolution"),                  QStringLiteral("Main Window"), {QStringLiteral("Alt+9"), Qt::ApplicationShortcut}},
     {QStringLiteral("10x Native Internal Resolution"),                 QStringLiteral("Main Window"), {QStringLiteral("Alt+0"), Qt::ApplicationShortcut}},
     {QStringLiteral("Advance Frame"),                                  QStringLiteral("Main Window"), {QStringLiteral("\\"), Qt::ApplicationShortcut}},
     {QStringLiteral("Auto Internal Resolution"),                       QStringLiteral("Main Window"), {QStringLiteral("Alt+A"), Qt::ApplicationShortcut}},
     {QStringLiteral("Capture Screenshot Then Save To File"),           QStringLiteral("Main Window"), {QStringLiteral("Ctrl+P"), Qt::ApplicationShortcut}},
     {QStringLiteral("Capture Screenshot Then Copy To Clipboard"),      QStringLiteral("Main Window"), {QStringLiteral("Ctrl+B"), Qt::ApplicationShortcut}},
     {QStringLiteral("Continue/Pause Emulation"),                       QStringLiteral("Main Window"), {QStringLiteral("F4"), Qt::WindowShortcut}},
     {QStringLiteral("Decrease Speed Limit"),                           QStringLiteral("Main Window"), {QStringLiteral("-"), Qt::ApplicationShortcut}},
     {QStringLiteral("Exit"),                                           QStringLiteral("Main Window"), {QStringLiteral("Ctrl+Q"), Qt::WindowShortcut}},
     {QStringLiteral("Exit Fullscreen"),                                QStringLiteral("Main Window"), {QStringLiteral("Esc"), Qt::WindowShortcut}},
     {QStringLiteral("Fullscreen"),                                     QStringLiteral("Main Window"), {QStringLiteral("F11"), Qt::WindowShortcut}},
     {QStringLiteral("Increase Speed Limit"),                           QStringLiteral("Main Window"), {QStringLiteral("+"), Qt::ApplicationShortcut}},
     {QStringLiteral("Load Amiibo"),                                    QStringLiteral("Main Window"), {QStringLiteral("F2"), Qt::ApplicationShortcut}},
     {QStringLiteral("Load File"),                                      QStringLiteral("Main Window"), {QStringLiteral("Ctrl+O"), Qt::WindowShortcut}},
     {QStringLiteral("Native Internal Resolution"),                     QStringLiteral("Main Window"), {QStringLiteral("Alt+1"), Qt::ApplicationShortcut}},
     {QStringLiteral("Remove Amiibo"),                                  QStringLiteral("Main Window"), {QStringLiteral("F3"), Qt::ApplicationShortcut}},
     {QStringLiteral("Restart Emulation"),                              QStringLiteral("Main Window"), {QStringLiteral("F6"), Qt::WindowShortcut}},
     {QStringLiteral("Stop Emulation"),                                 QStringLiteral("Main Window"), {QStringLiteral("F5"), Qt::WindowShortcut}},
     {QStringLiteral("Swap Screens"),                                   QStringLiteral("Main Window"), {QStringLiteral("F9"), Qt::WindowShortcut}},
     {QStringLiteral("Toggle Filter Bar"),                              QStringLiteral("Main Window"), {QStringLiteral("Ctrl+F"), Qt::WindowShortcut}},
     {QStringLiteral("Toggle Custom Ticks"),                            QStringLiteral("Main Window"), {QStringLiteral("Ctrl+T"), Qt::ApplicationShortcut}},
     {QStringLiteral("Toggle Frame Advancing"),                         QStringLiteral("Main Window"), {QStringLiteral("Ctrl+A"), Qt::ApplicationShortcut}},
     {QStringLiteral("Toggle Screen Layout"),                           QStringLiteral("Main Window"), {QStringLiteral("F10"), Qt::WindowShortcut}},
     {QStringLiteral("Toggle Speed Limit"),                             QStringLiteral("Main Window"), {QStringLiteral("Ctrl+Z"), Qt::ApplicationShortcut}},
     {QStringLiteral("Toggle Status Bar"),                              QStringLiteral("Main Window"), {QStringLiteral("Ctrl+S"), Qt::WindowShortcut}},
     {QStringLiteral("Toggle Texture Dumping"),                         QStringLiteral("Main Window"), {QStringLiteral("Ctrl+D"), Qt::ApplicationShortcut}},
     {QStringLiteral("Toggle Hardware Shader"),                         QStringLiteral("Main Window"), {QStringLiteral("Ctrl+H"), Qt::ApplicationShortcut}},
     {QStringLiteral("Toggle Custom Layout"),                           QStringLiteral("Main Window"), {QStringLiteral("Ctrl+C"), Qt::ApplicationShortcut}},
     {QStringLiteral("Toggle Custom Screen Refresh Rate"),              QStringLiteral("Main Window"), {QStringLiteral("Ctrl+R"), Qt::ApplicationShortcut}},
     {QStringLiteral("Change Custom Screen Refresh Rate"),              QStringLiteral("Main Window"), {QStringLiteral("Ctrl+Alt+R"), Qt::ApplicationShortcut}}}};
// clang-format on

void Config::ReadShortcutsValues() {
    qt_config->beginGroup(QStringLiteral("Shortcuts"));
    for (auto [name, group, shortcut] : default_hotkeys) {
        auto [keyseq, context] = shortcut;
        qt_config->beginGroup(group);
        qt_config->beginGroup(name);
        UISettings::values.shortcuts.push_back(
            {name,
             group,
             {ReadSetting(QStringLiteral("KeySeq"), keyseq).toString(),
              ReadSetting(QStringLiteral("Context"), context).toInt()}});
        qt_config->endGroup();
        qt_config->endGroup();
    }
    qt_config->endGroup();
}

void Config::SaveShortcutsValues() {
    qt_config->beginGroup(QStringLiteral("Shortcuts"));
    // Lengths of UISettings::values.shortcuts & default_hotkeys are same.
    // However, their ordering must also be the same.
    for (std::size_t i = 0; i < default_hotkeys.size(); i++) {
        auto [name, group, shortcut] = UISettings::values.shortcuts[i];
        qt_config->beginGroup(group);
        qt_config->beginGroup(name);
        WriteSetting(QStringLiteral("KeySeq"), shortcut.first, default_hotkeys[i].shortcut.first);
        WriteSetting(QStringLiteral("Context"), shortcut.second,
                     default_hotkeys[i].shortcut.second);
        qt_config->endGroup();
        qt_config->endGroup();
    }
    qt_config->endGroup();
}
