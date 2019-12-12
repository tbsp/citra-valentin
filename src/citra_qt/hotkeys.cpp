// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QKeySequence>
#include <QShortcut>
#include <QtGlobal>
#include "citra_qt/hotkeys.h"
#include "citra_qt/uisettings.h"

HotkeyRegistry::HotkeyRegistry() = default;
HotkeyRegistry::~HotkeyRegistry() = default;

void HotkeyRegistry::SaveHotkeys() {
    UISettings::values.shortcuts.clear();
    for (const std::pair<const QString, std::map<QString, HotkeyRegistry::Hotkey>>& group :
         hotkey_groups) {
        for (const std::pair<const QString, HotkeyRegistry::Hotkey>& hotkey : group.second) {
            UISettings::values.shortcuts.push_back(
                {hotkey.first, group.first,
                 UISettings::ContextualShortcut(hotkey.second.keyseq.toString(),
                                                hotkey.second.context)});
        }
    }
}

void HotkeyRegistry::LoadHotkeys() {
    // Make sure NOT to use a reference here because it would become invalid once we call
    // beginGroup()
    for (UISettings::Shortcut shortcut : UISettings::values.shortcuts) {
        Hotkey& hotkey = hotkey_groups[shortcut.group][shortcut.name];
        if (!shortcut.shortcut.first.isEmpty()) {
            hotkey.keyseq =
                QKeySequence::fromString(shortcut.shortcut.first, QKeySequence::NativeText);
            hotkey.context = static_cast<Qt::ShortcutContext>(shortcut.shortcut.second);
        }
        if (hotkey.shortcut) {
            hotkey.shortcut->disconnect();
            hotkey.shortcut->setKey(hotkey.keyseq);
        }
    }
}

QShortcut* HotkeyRegistry::GetHotkey(const QString& group, const QString& action, QWidget* widget) {
    Hotkey& hotkey = hotkey_groups[group][action];

    if (!hotkey.shortcut) {
        hotkey.shortcut = new QShortcut(hotkey.keyseq, widget, nullptr, nullptr, hotkey.context);
    }

    return hotkey.shortcut;
}

QKeySequence HotkeyRegistry::GetKeySequence(const QString& group, const QString& action) {
    Hotkey& hotkey = hotkey_groups[group][action];
    return hotkey.keyseq;
}

Qt::ShortcutContext HotkeyRegistry::GetShortcutContext(const QString& group,
                                                       const QString& action) {
    Hotkey& hotkey = hotkey_groups[group][action];
    return hotkey.context;
}
