// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <QWidget>

namespace Ui {
class ConfigureCustomLayout;
} // namespace Ui

class ConfigureCustomLayout : public QWidget {
    Q_OBJECT

public:
    explicit ConfigureCustomLayout(QWidget* parent = nullptr);
    ~ConfigureCustomLayout();

    void SetConfiguration();
    void ApplyConfiguration();

private:
    void updateShaders(bool anaglyph);

    Ui::ConfigureCustomLayout* ui;
};
