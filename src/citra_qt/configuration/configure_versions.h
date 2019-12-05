// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <memory>
#include <QWidget>
#include "common/web_result.h"

namespace Ui {
class ConfigureVersions;
} // namespace Ui

template <typename>
class QFutureWatcher;

class QDialog;

#ifdef _WIN32
#include <QString>
#include "semver.hpp"

constexpr int VersionRole = Qt::UserRole;
constexpr int PathRole = Qt::UserRole + 1;
#endif

class ConfigureVersions : public QWidget {
    Q_OBJECT

public:
    explicit ConfigureVersions(QWidget* parent = nullptr);
    ~ConfigureVersions();

private:
    void Initialize();
    void UpdateInstalledVersions();

    std::unique_ptr<QFutureWatcher<Common::WebResult>> fetch_recent_releases_watcher;
    std::unique_ptr<QDialog> fetching_recent_releases_dialog;

#ifdef _WIN32
    QString install_dir;
#endif

    std::unique_ptr<Ui::ConfigureVersions> ui;
};

#ifdef _WIN32
Q_DECLARE_METATYPE(semver::version);
#endif
