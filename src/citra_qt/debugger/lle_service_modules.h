// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <QDockWidget>

class LleServiceModulesWidget : public QDockWidget {
    Q_OBJECT

public:
    explicit LleServiceModulesWidget(QWidget* parent = nullptr);
    ~LleServiceModulesWidget();
};
