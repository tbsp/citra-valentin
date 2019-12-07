// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <memory>
#include <vector>
#include <QPoint>
#include <QWidget>
#include "common/version.h"

namespace Ui {
class ConfigureReleases;
} // namespace Ui

namespace httplib {
struct Response;
} // namespace httplib

template <typename>
class QFutureWatcher;

class QProgressDialog;

#ifdef _WIN32
#include <QString>
#endif

class ConfigureReleases : public QWidget {
    Q_OBJECT

public:
    explicit ConfigureReleases(QWidget* parent = nullptr);
    ~ConfigureReleases();

private slots:
    void FetchReleases();
    void FetchWindowsUpdaterReleases();

    void ShowContextMenuForReleases(const QPoint& position);
    void ShowContextMenuForWindowsUpdater(const QPoint& position);

private:
    std::unique_ptr<QFutureWatcher<std::vector<std::shared_ptr<httplib::Response>>>>
        http_responses_future_watcher;
    std::unique_ptr<QProgressDialog> progress_dialog;

#ifdef _WIN32
    QString install_dir;
#endif

    std::unique_ptr<Ui::ConfigureReleases> ui;
};

Q_DECLARE_METATYPE(semver::version);
