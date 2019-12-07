// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <optional>
#include <QDesktopServices>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QMenu>
#include <QMessageBox>
#ifdef _WIN32
#include <QProcess>
#endif
#include <QProgressDialog>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QStringList>
#include <QTimer>
#include <QUrl>
#include <QtConcurrent/QtConcurrentRun>
#include <fmt/format.h>
#include <httplib.h>
#include <json.hpp>
#include "citra_qt/configuration/configure_releases.h"
#include "common/logging/log.h"
#include "ui_configure_releases.h"

const QRegularExpression HTTP_LINK_REGEX(QStringLiteral("<(?P<link>.*)>; rel=\"(?P<rel>[^\"]*)\""));

// Release data roles
constexpr int VersionRole = Qt::UserRole;
constexpr int AssetsRole = Qt::UserRole + 1;
#ifdef _WIN32
constexpr int PathRole = Qt::UserRole + 2;
#endif

ConfigureReleases::ConfigureReleases(QWidget* parent)
    : QWidget(parent), ui(std::make_unique<Ui::ConfigureReleases>()) {
    ui->setupUi(this);

    ui->releases->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->cvu_releases->setContextMenuPolicy(Qt::CustomContextMenu);

    qRegisterMetaType<std::vector<std::shared_ptr<httplib::Response>>>(
        "std::vector<std::shared_ptr<httplib::Response>>");

#ifdef _WIN32
    // Set the installation directory
    char* cvu_install_dir = getenv("CVU_INSTALL_DIR");
    install_dir = cvu_install_dir == nullptr
                      ? QStandardPaths::writableLocation(QStandardPaths::HomeLocation) +
                            "/AppData/Local/citra-valentin"
                      : QString::fromLocal8Bit(cvu_install_dir);

    ui->cvu_install_dir->setText(QStringLiteral("Installation Directory: %1").arg(install_dir));
    connect(ui->open_cvu_install_dir, &QPushButton::clicked, this, [this] {
        QDir dir(install_dir);
        if (!dir.exists() && !dir.mkdir(install_dir)) {
            QMessageBox::critical(this, QStringLiteral("Error"),
                                  QStringLiteral("Failed to create %1").arg(install_dir));
        }

        QDesktopServices::openUrl(QUrl::fromLocalFile(install_dir));
    });
#else
    ui->cvu_install_dir_group->hide();
#endif

    connect(ui->fetch_releases, &QPushButton::clicked, this, &ConfigureReleases::FetchReleases);
    connect(ui->fetch_cvu_releases, &QPushButton::clicked, this,
            &ConfigureReleases::FetchWindowsUpdaterReleases);

    connect(ui->releases, &QListWidget::customContextMenuRequested, this,
            &ConfigureReleases::ShowContextMenuForReleases);
    connect(ui->cvu_releases, &QListWidget::customContextMenuRequested, this,
            &ConfigureReleases::ShowContextMenuForWindowsUpdater);
}

ConfigureReleases::~ConfigureReleases() = default;

void ConfigureReleases::FetchReleases() {
    http_responses_future_watcher =
        std::make_unique<QFutureWatcher<std::vector<std::shared_ptr<httplib::Response>>>>();
    progress_dialog =
        std::make_unique<QProgressDialog>(QStringLiteral("Fetching releases"), QString(), 0, 0,
                                          this, Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
    progress_dialog->setWindowTitle(QStringLiteral("Fetching releases"));

    connect(
        http_responses_future_watcher.get(),
        &QFutureWatcher<std::vector<std::shared_ptr<httplib::Response>>>::finished, this, [this] {
            const std::vector<std::shared_ptr<httplib::Response>> responses =
                http_responses_future_watcher->result();

            http_responses_future_watcher.reset();

            progress_dialog.reset();

            if (responses.empty()) {
                QMessageBox::critical(this, QStringLiteral("Error"),
                                      QStringLiteral("Error while fetching the releases."));
                return;
            }

            ui->releases->clear();

            for (const std::shared_ptr<httplib::Response>& response : responses) {
                const nlohmann::json json = nlohmann::json::parse(response->body);
                for (const nlohmann::json release : json) {
                    // Validate the version
                    const std::optional<semver::version> opt =
                        semver::from_string_noexcept(release["tag_name"].get<std::string>());
                    if (opt.has_value()) {
                        const semver::version version = *opt;
                        QString text = QString::fromStdString(version.to_string());
                        if (version == Version::citra_valentin) {
                            text.append(QStringLiteral(" (running)"));
                        }

                        QListWidgetItem* item = new QListWidgetItem(QString());

                        item->setData(VersionRole, QVariant::fromValue<semver::version>(version));

                        QStringList assets;
                        for (const nlohmann::json asset : release["assets"]) {
                            assets << QString::fromStdString(asset["name"].get<std::string>());
                        }

                        item->setData(AssetsRole, assets);

#ifdef _WIN32
                        QString path =
                            QStringLiteral("%1/citra-valentin-windows-%2")
                                .arg(install_dir, QString::fromStdString(version.to_string()));

                        // Add -disabled if path doesn't exist
                        if (!QDir(path).exists()) {
                            path.append(QStringLiteral("-disabled"));
                        }

                        // Check again.
                        if (QDir(path).exists()) {
                            item->setData(PathRole, path);
                            text.append(QStringLiteral(" (installed)"));
                        }
#endif

                        item->setText(text);

                        ui->releases->addItem(item);
                    }
                }
            }
        });
    QFuture<std::vector<std::shared_ptr<httplib::Response>>> future = QtConcurrent::run([] {
        httplib::SSLClient client("api.github.com", 443);
        std::vector<std::shared_ptr<httplib::Response>> responses;
        const std::function<bool(const std::string&)> Next = [&Next, &client,
                                                              &responses](const std::string& path) {
            std::shared_ptr<httplib::Response> response = client.Get(path.c_str());
            if (response != nullptr && response->status == 200) {
                const auto content_type = response->headers.find("Content-Type");
                if (content_type != response->headers.end() &&
                    content_type->second.find("application/json") == 0) {
                    responses.push_back(response);

                    const auto link = response->headers.find("Link");
                    if (link != response->headers.end()) {
                        const QString qlink = QString::fromStdString(link->second);
                        const QStringList links = qlink.split(", ");

                        // Find the next link
                        for (const QString& link : links) {
                            const QRegularExpressionMatch match = HTTP_LINK_REGEX.match(link);
                            if (match.hasMatch()) {
                                const QString rel = match.captured(QStringLiteral("rel"));
                                if (rel == QStringLiteral("next")) {
                                    const QString link = match.captured(QStringLiteral("link"));
                                    const QUrl url(link);
                                    // Go to next page
                                    Next(static_cast<QString>(url.path() + QLatin1Char{'?'} +
                                                              url.query())
                                             .toStdString());
                                    break;
                                }
                            }
                        }
                    }

                    return true;
                }
            }
            return false;
        };
        Next("/repos/vvanelslande/citra/releases");
        return responses;
    });
    http_responses_future_watcher->setFuture(future);
    progress_dialog->exec();
}

void ConfigureReleases::ShowContextMenuForReleases(const QPoint& position) {
    QListWidgetItem* item = ui->releases->currentItem();
    if (item == nullptr) {
        return;
    }

    const semver::version version = item->data(VersionRole).value<semver::version>();
    QMenu menu(QString::fromStdString(version.to_string()), this);

#ifdef _WIN32
    if (!item->data(PathRole).toString().isEmpty()) {
        QAction* action_start = new QAction(QStringLiteral("Start"), &menu);
        connect(action_start, &QAction::triggered, this, [this, item] {
            QProcess* process = new QProcess(this);
            process->startDetached(
                QStringLiteral("%1/citra-valentin-qt.exe").arg(item->data(PathRole).toString()),
                QStringList());
        });
        menu.addAction(action_start);
    }
#endif

    const auto DownloadAsset = [this, version](const std::string& name) {
        const QString path = QFileDialog::getSaveFileName(
            this, QStringLiteral("Save %1").arg(QString::fromStdString(name)),
            QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) +
                QStringLiteral("/%1").arg(QString::fromStdString(name)));

        if (path.isEmpty()) {
            return;
        }

        http_responses_future_watcher =
            std::make_unique<QFutureWatcher<std::vector<std::shared_ptr<httplib::Response>>>>();
        progress_dialog = std::make_unique<QProgressDialog>(
            QStringLiteral("Downloading Asset"), QString(), 0, 100, this,
            Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
        progress_dialog->setWindowTitle(QStringLiteral("Downloading Asset"));
        connect(http_responses_future_watcher.get(),
                &QFutureWatcher<std::vector<std::shared_ptr<httplib::Response>>>::finished, this,
                [this, path] {
                    const std::vector<std::shared_ptr<httplib::Response>> responses =
                        http_responses_future_watcher->result();

                    http_responses_future_watcher.reset();
                    progress_dialog.reset();

                    if (responses.empty()) {
                        QMessageBox::critical(this, QStringLiteral("Error"),
                                              QStringLiteral("Error while downloading the asset."));
                        return;
                    } else {
                        QFile file(path);
                        file.open(QFile::WriteOnly);
                        if (file.write(responses[0]->body.data(), responses[0]->body.size()) ==
                            responses[0]->body.size()) {
                            QMessageBox::information(this, QStringLiteral("File Saved"),
                                                     QStringLiteral("File saved"));
                        } else {
                            if (file.remove()) {
                                QMessageBox::critical(
                                    this, QStringLiteral("Error"),
                                    QStringLiteral("Write failed. The file has been deleted."));
                            } else {
                                QMessageBox::critical(
                                    this, QStringLiteral("Error"),
                                    QStringLiteral("Write failed. The file has not been deleted."));
                            }
                        }
                    }
                });
        QFuture<std::vector<std::shared_ptr<httplib::Response>>> future =
            QtConcurrent::run([this, &name, version] {
                httplib::SSLClient client("github.com", 443);
                client.follow_location(true);

                std::shared_ptr<httplib::Response> response = client.Get(
                    fmt::format("/vvanelslande/citra/releases/download/{}/{}", version.to_string(),
                                name)
                        .c_str(),
                    [this](u64 current, u64 total) {
                        QTimer::singleShot(0, qApp, [this, current, total] {
                            progress_dialog->setValue(static_cast<int>(
                                100 * (static_cast<double>(current) / static_cast<double>(total))));
                        });

                        return true;
                    });

                if (response == nullptr || response->status != 200) {
                    return std::vector<std::shared_ptr<httplib::Response>>();
                }

                return std::vector<std::shared_ptr<httplib::Response>>{response};
            });
        http_responses_future_watcher->setFuture(future);
        progress_dialog->exec();
    };

    QAction action_open_release(QStringLiteral("Open Release"), &menu);
    connect(&action_open_release, &QAction::triggered, this, [this, item, version] {
        QDesktopServices::openUrl(
            QUrl(QStringLiteral("https://github.com/vvanelslande/citra/releases/tag/%1")
                     .arg(QString::fromStdString(version.to_string()))));
    });
    menu.addAction(&action_open_release);

    // There's no changelog for versions before 2.10.1 currently.
    if (version >= semver::version{2, 10, 1}) {
        QAction* action_open_changelog = new QAction(QStringLiteral("Open Changelog"), &menu);
        connect(action_open_changelog, &QAction::triggered, this, [this, item, version] {
            QDesktopServices::openUrl(QUrl(
                QStringLiteral("https://github.com/vvanelslande/citra/blob/"
                               "master/changelog.md#%1")
                    .arg(QString::fromStdString(version.to_string()).remove(QLatin1Char{'.'}))));
        });
        menu.addAction(action_open_changelog);
    }

#ifdef _WIN32
    if (!item->data(PathRole).toString().isEmpty()) {
        QAction* action_open_location = new QAction(QStringLiteral("Open Location"), &menu);
        connect(action_open_location, &QAction::triggered, this, [this, item] {
            QDesktopServices::openUrl(QUrl::fromLocalFile(item->data(PathRole).toString()));
        });
        menu.addAction(action_open_location);
    }
#endif

#ifdef _WIN32
    if (!item->data(PathRole).toString().isEmpty() && version != Version::citra_valentin) {
        QAction* action_delete = new QAction(QStringLiteral("Delete"), &menu);
        connect(action_delete, &QAction::triggered, this, [this, item] {
            QDir dir(item->data(PathRole).toString());
            dir.removeRecursively();
            FetchReleases();
        });
        menu.addAction(action_delete);
    }
#endif

    const QStringList assets = item->data(AssetsRole).toStringList();
    if (!assets.isEmpty()) {
        menu.addSeparator();

        for (const QString& asset : assets) {
            QAction* action = new QAction(QStringLiteral("Download %1").arg(asset), &menu);
            connect(action, &QAction::triggered, this,
                    [asset = asset.toStdString(), DownloadAsset] { DownloadAsset(asset); });
            menu.addAction(action);
        }
    }

    menu.exec(ui->releases->mapToGlobal(position));
}

void ConfigureReleases::FetchWindowsUpdaterReleases() {
    http_responses_future_watcher =
        std::make_unique<QFutureWatcher<std::vector<std::shared_ptr<httplib::Response>>>>();
    progress_dialog =
        std::make_unique<QProgressDialog>(QStringLiteral("Fetching releases"), QString(), 0, 0,
                                          this, Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
    progress_dialog->setWindowTitle(QStringLiteral("Fetching releases"));

    connect(
        http_responses_future_watcher.get(),
        &QFutureWatcher<std::vector<std::shared_ptr<httplib::Response>>>::finished, this, [this] {
            const std::vector<std::shared_ptr<httplib::Response>> responses =
                http_responses_future_watcher->result();

            http_responses_future_watcher.reset();

            progress_dialog.reset();

            if (responses.empty()) {
                QMessageBox::critical(this, QStringLiteral("Error"),
                                      QStringLiteral("Error while fetching the releases."));
                return;
            }

            ui->cvu_releases->clear();

            for (const std::shared_ptr<httplib::Response>& response : responses) {
                const nlohmann::json json = nlohmann::json::parse(response->body);
                for (const nlohmann::json release : json) {
                    // Validate the version
                    const std::optional<semver::version> opt =
                        semver::from_string_noexcept(release["tag_name"].get<std::string>());
                    if (opt.has_value()) {
                        const semver::version version = *opt;

                        QListWidgetItem* item =
                            new QListWidgetItem(QString::fromStdString(version.to_string()));

                        item->setData(VersionRole, QVariant::fromValue<semver::version>(version));

                        QStringList assets;
                        for (const nlohmann::json asset : release["assets"]) {
                            assets << QString::fromStdString(asset["name"].get<std::string>());
                        }

                        item->setData(AssetsRole, assets);

                        ui->cvu_releases->addItem(item);
                    }
                }
            }
        });
    QFuture<std::vector<std::shared_ptr<httplib::Response>>> future = QtConcurrent::run([] {
        httplib::SSLClient client("api.github.com", 443);
        std::vector<std::shared_ptr<httplib::Response>> responses;
        const std::function<bool(const std::string&)> Next = [&Next, &client,
                                                              &responses](const std::string& path) {
            std::shared_ptr<httplib::Response> response = client.Get(path.c_str());
            if (response != nullptr && response->status == 200) {
                const auto content_type = response->headers.find("Content-Type");
                if (content_type != response->headers.end() &&
                    content_type->second.find("application/json") == 0) {
                    responses.push_back(response);

                    const auto link = response->headers.find("Link");
                    if (link != response->headers.end()) {
                        const QString qlink = QString::fromStdString(link->second);
                        const QStringList links = qlink.split(", ");

                        // Find the next link
                        for (const QString& link : links) {
                            const QRegularExpressionMatch match = HTTP_LINK_REGEX.match(link);
                            if (match.hasMatch()) {
                                const QString rel = match.captured(QStringLiteral("rel"));
                                if (rel == QStringLiteral("next")) {
                                    const QString link = match.captured(QStringLiteral("link"));
                                    const QUrl url(link);
                                    // Go to next page
                                    Next(static_cast<QString>(url.path() + QLatin1Char{'?'} +
                                                              url.query())
                                             .toStdString());
                                    break;
                                }
                            }
                        }
                    }

                    return true;
                }
            }
            return false;
        };
        Next("/repos/vvanelslande/cvu/releases");
        return responses;
    });
    http_responses_future_watcher->setFuture(future);
    progress_dialog->exec();
}

void ConfigureReleases::ShowContextMenuForWindowsUpdater(const QPoint& position) {
    QListWidgetItem* item = ui->cvu_releases->currentItem();
    if (item == nullptr) {
        return;
    }

    const semver::version version = item->data(VersionRole).value<semver::version>();
    QMenu menu(QString::fromStdString(version.to_string()), this);

    const auto DownloadAsset = [this, version](const std::string& name) {
        const QString path = QFileDialog::getSaveFileName(
            this, QStringLiteral("Save %1").arg(QString::fromStdString(name)),
            QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) +
                QStringLiteral("/%1").arg(QString::fromStdString(name)));

        if (path.isEmpty()) {
            return;
        }

        http_responses_future_watcher =
            std::make_unique<QFutureWatcher<std::vector<std::shared_ptr<httplib::Response>>>>();
        progress_dialog = std::make_unique<QProgressDialog>(
            QStringLiteral("Downloading Asset"), QString(), 0, 100, this,
            Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
        progress_dialog->setWindowTitle(QStringLiteral("Downloading Asset"));
        connect(http_responses_future_watcher.get(),
                &QFutureWatcher<std::vector<std::shared_ptr<httplib::Response>>>::finished, this,
                [this, path] {
                    const std::vector<std::shared_ptr<httplib::Response>> responses =
                        http_responses_future_watcher->result();

                    http_responses_future_watcher.reset();
                    progress_dialog.reset();

                    if (responses.empty()) {
                        QMessageBox::critical(this, QStringLiteral("Error"),
                                              QStringLiteral("Error while downloading the asset."));
                        return;
                    } else {
                        QFile file(path);
                        file.open(QFile::WriteOnly);
                        if (file.write(responses[0]->body.data(), responses[0]->body.size()) ==
                            responses[0]->body.size()) {
                            QMessageBox::information(this, QStringLiteral("File Saved"),
                                                     QStringLiteral("File saved"));
                        } else {
                            if (file.remove()) {
                                QMessageBox::critical(
                                    this, QStringLiteral("Error"),
                                    QStringLiteral("Write failed. The file has been deleted."));
                            } else {
                                QMessageBox::critical(
                                    this, QStringLiteral("Error"),
                                    QStringLiteral("Write failed. The file has not been deleted."));
                            }
                        }
                    }
                });
        QFuture<std::vector<std::shared_ptr<httplib::Response>>> future =
            QtConcurrent::run([this, &name, version] {
                httplib::SSLClient client("github.com", 443);
                client.follow_location(true);

                std::shared_ptr<httplib::Response> response = client.Get(
                    fmt::format("/vvanelslande/cvu/releases/download/{}/{}", version.to_string(),
                                name)
                        .c_str(),
                    [this](u64 current, u64 total) {
                        QTimer::singleShot(0, qApp, [this, current, total] {
                            progress_dialog->setValue(static_cast<int>(
                                100 * (static_cast<double>(current) / static_cast<double>(total))));
                        });

                        return true;
                    });

                if (response == nullptr || response->status != 200) {
                    return std::vector<std::shared_ptr<httplib::Response>>();
                }

                return std::vector<std::shared_ptr<httplib::Response>>{response};
            });
        http_responses_future_watcher->setFuture(future);
        progress_dialog->exec();
    };

    QAction action_open_release(QStringLiteral("Open Release"), &menu);
    connect(&action_open_release, &QAction::triggered, this, [this, item, version] {
        QDesktopServices::openUrl(
            QUrl(QStringLiteral("https://github.com/vvanelslande/cvu/releases/tag/%1")
                     .arg(QString::fromStdString(version.to_string()))));
    });
    menu.addAction(&action_open_release);

    QAction* action_open_changelog = new QAction(QStringLiteral("Open Changelog"), &menu);
    connect(action_open_changelog, &QAction::triggered, this, [this, item, version] {
        QDesktopServices::openUrl(
            QUrl(QStringLiteral("https://github.com/vvanelslande/cvu/blob/master/changelog.md#%1")
                     .arg(QString::fromStdString(version.to_string()).remove(QLatin1Char{'.'}))));
    });
    menu.addAction(action_open_changelog);

    const QStringList assets = item->data(AssetsRole).toStringList();
    if (!assets.isEmpty()) {
        menu.addSeparator();

        for (const QString& asset : assets) {
            QAction* action = new QAction(QStringLiteral("Download %1").arg(asset), &menu);
            connect(action, &QAction::triggered, this,
                    [asset = asset.toStdString(), DownloadAsset] { DownloadAsset(asset); });
            menu.addAction(action);
        }
    }

    menu.exec(ui->cvu_releases->mapToGlobal(position));
}
