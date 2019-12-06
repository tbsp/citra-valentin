// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QDesktopServices>
#include <QFile>
#include <QFileDialog>
#include <QFutureWatcher>
#include <QMenu>
#include <QMessageBox>
#include <QProgressDialog>
#include <QStandardPaths>
#include <QTimer>
#include <QUrl>
#include <QtConcurrent/QtConcurrentRun>
#include <fmt/format.h>
#include <httplib.h>
#include <json.hpp>
#include "citra_qt/configuration/configure_versions.h"
#include "ui_configure_versions.h"
#include "web_service/web_backend.h"

ConfigureVersions::ConfigureVersions(QWidget* parent)
    : QWidget(parent), ui(std::make_unique<Ui::ConfigureVersions>()) {
    ui->setupUi(this);

    ui->versions_installed_using_cvu_group->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->recent_releases->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(
        ui->recent_releases, &QListWidget::customContextMenuRequested, this,
        [this](const QPoint& position) {
            QListWidgetItem* item = ui->recent_releases->currentItem();
            if (item == nullptr) {
                return;
            }

            const semver::version version(item->text().toStdString());
            QMenu menu(item->text(), this);

            QAction action_open_release(QStringLiteral("Open Release"), this);
            connect(&action_open_release, &QAction::triggered, this, [this, item] {
                QDesktopServices::openUrl(
                    QUrl(QStringLiteral("https://github.com/vvanelslande/citra/releases/tag/%1")
                             .arg(item->text())));
            });
            menu.addAction(&action_open_release);

            QAction action_open_changelog(QStringLiteral("Open Changelog"), this);
            // There's no changelog for versions before 2.10.1 currently.
            if (version >= semver::version{2, 10, 1}) {
                connect(&action_open_changelog, &QAction::triggered, this, [this, item] {
                    QDesktopServices::openUrl(
                        QUrl(QStringLiteral("https://github.com/vvanelslande/citra/blob/"
                                            "master/changelog.md#%1")
                                 .arg(item->text().remove(QLatin1Char{'.'}))));
                });

                menu.addAction(&action_open_changelog);
            }

            const auto DownloadBuild = [this, item, &version](const std::string& os) {
                const QString path = QFileDialog::getSaveFileName(
                    this,
                    QStringLiteral("Save citra-valentin-%1-%2.tar.gz")
                        .arg(QString::fromStdString(os),
                             QString::fromStdString(version.to_string())),
                    QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) +
                        QStringLiteral("/citra-valentin-%1-%2.tar.gz")
                            .arg(QString::fromStdString(os),
                                 QString::fromStdString(version.to_string())),
                    QStringLiteral("Gzipped Tarball (*.tar.gz);;All Files (*.*)"));

                if (path.isEmpty()) {
                    return;
                }

                http_future_watcher =
                    std::make_unique<QFutureWatcher<std::shared_ptr<httplib::Response>>>();
                progress_dialog = std::make_unique<QProgressDialog>(
                    QStringLiteral("Downloading Build"), QString(), 0, 100, this,
                    Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
                progress_dialog->setWindowTitle(QStringLiteral("Downloading Build"));
                connect(
                    http_future_watcher.get(),
                    &QFutureWatcher<std::shared_ptr<httplib::Response>>::finished, this,
                    [this, path] {
                        const std::shared_ptr<httplib::Response> result =
                            http_future_watcher->result();

                        if (result != nullptr) {
                            QFile file(path);
                            file.open(QFile::WriteOnly);
                            if (file.write(result->body.data(), result->body.size()) ==
                                result->body.size()) {
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
                                        QStringLiteral(
                                            "Write failed. The file has not been deleted."));
                                }
                            }
                        }

                        http_future_watcher.reset();
                        progress_dialog.reset();
                    });
                QFuture<std::shared_ptr<httplib::Response>> future = QtConcurrent::run(
                    [this, &version, &os]() -> std::shared_ptr<httplib::Response> {
                        httplib::SSLClient client("github.com", 443);
                        client.follow_location(true);

                        std::shared_ptr<httplib::Response> response =
                            client.Get(fmt::format("/vvanelslande/citra/releases/download/{0}/"
                                                   "citra-valentin-{1}-{0}.tar.gz",
                                                   version.to_string(), os)
                                           .c_str(),
                                       [this](u64 current, u64 total) {
                                           QTimer::singleShot(0, qApp, [this, current, total] {
                                               progress_dialog->setValue(static_cast<int>(
                                                   100 * (static_cast<double>(current) /
                                                          static_cast<double>(total))));
                                           });

                                           return true;
                                       });

                        if (response == nullptr) {
                            return nullptr;
                        }

                        return response;
                    });
                http_future_watcher->setFuture(future);
                progress_dialog->exec();
            };

            QAction action_download_linux_build(QStringLiteral("Download Linux Build"), this);
            connect(&action_download_linux_build, &QAction::triggered, this,
                    [DownloadBuild] { DownloadBuild("linux"); });
            menu.addAction(&action_download_linux_build);

            QAction action_download_windows_build(QStringLiteral("Download Windows Build"), this);
            connect(&action_download_windows_build, &QAction::triggered, this,
                    [DownloadBuild] { DownloadBuild("windows"); });
            menu.addAction(&action_download_windows_build);

            menu.exec(ui->recent_releases->mapToGlobal(position));
        });

    connect(ui->recent_releases_button, &QPushButton::clicked, this, [this] {
        web_result_future_watcher = std::make_unique<QFutureWatcher<Common::WebResult>>();
        progress_dialog = std::make_unique<QProgressDialog>(
            QStringLiteral("Fetching recent releases"), QString(), 0, 0, this,
            Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
        progress_dialog->setWindowTitle(QStringLiteral("Fetching recent releases"));

        connect(web_result_future_watcher.get(), &QFutureWatcher<Common::WebResult>::finished, this,
                [this] {
                    const Common::WebResult result = web_result_future_watcher->result();

                    web_result_future_watcher.reset();
                    ui->recent_releases_button->setEnabled(true);

                    progress_dialog.reset();

                    if (result.result_code == Common::WebResult::Code::Success) {
                        if (!ui->recent_releases->isEnabled()) {
                            ui->recent_releases->setEnabled(true);
                            ui->recent_releases_button->setText(QStringLiteral("Update"));
                        }

                        ui->recent_releases->clear();

                        const nlohmann::json json = nlohmann::json::parse(result.returned_data);
                        for (std::size_t i = 0; i < json.size(); i++) {
                            const nlohmann::json release = json[i];

                            ui->recent_releases->addItem(
                                QString::fromStdString(release["tag_name"]));
                        }
                    }
                });
        QFuture<Common::WebResult> future = QtConcurrent::run([] {
            WebService::Client client("https://api.github.com", "", "");
            return client.GetJson("/repos/vvanelslande/citra/releases", true);
        });
        web_result_future_watcher->setFuture(future);
        ui->recent_releases_button->setEnabled(false);
        progress_dialog->exec();
    });

    Initialize();
    UpdateInstalledVersions();
}

ConfigureVersions::~ConfigureVersions() = default;
