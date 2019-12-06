// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <memory>
#include <QWidget>
#include "common/version.h"
#include "common/web_result.h"

namespace Ui {
class ConfigureVersions;
} // namespace Ui

namespace httplib {
struct Response;
} // namespace httplib

template <typename>
class QFutureWatcher;

class QProgressDialog;

#ifdef _WIN32
#include <QString>

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

    std::unique_ptr<QFutureWatcher<Common::WebResult>> web_result_future_watcher;
    std::unique_ptr<QFutureWatcher<std::shared_ptr<httplib::Response>>> http_future_watcher;
    std::unique_ptr<QProgressDialog> progress_dialog;

#ifdef _WIN32
    QString install_dir;
#endif

    std::unique_ptr<Ui::ConfigureVersions> ui;
};

#ifdef _WIN32
Q_DECLARE_METATYPE(semver::version);
#endif
