// Copyright 2015 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <QAbstractItemModel>
#include <QDockWidget>
#include <QTimer>
#include "common/microprofile.h"

class MicroprofileDialog : public QWidget {
    Q_OBJECT

public:
    explicit MicroprofileDialog(QWidget* parent = nullptr);

    /// Returns a QAction that can be used to toggle visibility of this dialog.
    QAction* toggleViewAction();

protected:
    void showEvent(QShowEvent* ev) override;
    void hideEvent(QHideEvent* ev) override;

private:
    QAction* toggle_view_action = nullptr;
};
