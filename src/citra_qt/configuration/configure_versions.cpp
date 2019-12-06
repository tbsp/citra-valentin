// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QDesktopServices>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QMenu>
#include <QMessageBox>
#include <QProgressDialog>
#include <QStandardPaths>
#include <QStringList>
#include <QTimer>
#include <QUrl>
#include <QtConcurrent/QtConcurrentRun>
#include <fmt/format.h>
#include <httplib.h>
#include <json.hpp>
#include "citra_qt/configuration/configure_versions.h"
#include "ui_configure_versions.h"

const QRegularExpression HTTP_LINK_REGEX(QStringLiteral("<(?P<link>.*)>; rel=\"(?P<rel>[^\"]*)\""));
constexpr int AssetsRole = Qt::UserRole;

ConfigureVersions::ConfigureVersions(QWidget* parent)
    : QWidget(parent), ui(std::make_unique<Ui::ConfigureVersions>()) {
    ui->setupUi(this);

    ui->versions_installed_using_cvu->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->releases->setContextMenuPolicy(Qt::CustomContextMenu);

    qRegisterMetaType<std::vector<std::shared_ptr<httplib::Response>>>(
        "std::vector<std::shared_ptr<httplib::Response>>");

    connect(
        ui->releases, &QListWidget::customContextMenuRequested, this,
        [this](const QPoint& position) {
            QListWidgetItem* item = ui->releases->currentItem();
            if (item == nullptr) {
                return;
            }

            const semver::version version(item->text().toStdString());
            QMenu menu(item->text(), this);

            const auto DownloadAsset = [this, version](const std::string& name) {
                const QString path = QFileDialog::getSaveFileName(
                    this, QStringLiteral("Save %1").arg(QString::fromStdString(name)),
                    QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) +
                        QStringLiteral("/%1").arg(QString::fromStdString(name)));

                if (path.isEmpty()) {
                    return;
                }

                http_responses_future_watcher = std::make_unique<
                    QFutureWatcher<std::vector<std::shared_ptr<httplib::Response>>>>();
                progress_dialog = std::make_unique<QProgressDialog>(
                    QStringLiteral("Downloading Asset"), QString(), 0, 100, this,
                    Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
                progress_dialog->setWindowTitle(QStringLiteral("Downloading Asset"));
                connect(
                    http_responses_future_watcher.get(),
                    &QFutureWatcher<std::vector<std::shared_ptr<httplib::Response>>>::finished,
                    this, [this, path] {
                        const std::vector<std::shared_ptr<httplib::Response>> responses =
                            http_responses_future_watcher->result();

                        http_responses_future_watcher.reset();
                        progress_dialog.reset();

                        if (responses.empty()) {
                            QMessageBox::critical(
                                this, QStringLiteral("Error"),
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
                                        QStringLiteral(
                                            "Write failed. The file has not been deleted."));
                                }
                            }
                        }
                    });
                QFuture<std::vector<std::shared_ptr<httplib::Response>>> future =
                    QtConcurrent::run([this, &name, version] {
                        httplib::SSLClient client("github.com", 443);
                        client.follow_location(true);

                        std::shared_ptr<httplib::Response> response =
                            client.Get(fmt::format("/vvanelslande/citra/releases/download/{}/{}",
                                                   version.to_string(), name)
                                           .c_str(),
                                       [this](u64 current, u64 total) {
                                           QTimer::singleShot(0, qApp, [this, current, total] {
                                               progress_dialog->setValue(static_cast<int>(
                                                   100 * (static_cast<double>(current) /
                                                          static_cast<double>(total))));
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
            connect(&action_open_release, &QAction::triggered, this, [this, item] {
                QDesktopServices::openUrl(
                    QUrl(QStringLiteral("https://github.com/vvanelslande/citra/releases/tag/%1")
                             .arg(item->text())));
            });
            menu.addAction(&action_open_release);

            // There's no changelog for versions before 2.10.1 currently.
            if (version >= semver::version{2, 10, 1}) {
                QAction* action_open_changelog =
                    new QAction(QStringLiteral("Open Changelog"), &menu);

                connect(action_open_changelog, &QAction::triggered, this, [this, item] {
                    QDesktopServices::openUrl(
                        QUrl(QStringLiteral("https://github.com/vvanelslande/citra/blob/"
                                            "master/changelog.md#%1")
                                 .arg(item->text().remove(QLatin1Char{'.'}))));
                });

                menu.addAction(action_open_changelog);
            }

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
        });

    connect(ui->releases_button, &QPushButton::clicked, this, [this] {
        http_responses_future_watcher =
            std::make_unique<QFutureWatcher<std::vector<std::shared_ptr<httplib::Response>>>>();
        progress_dialog =
            std::make_unique<QProgressDialog>(QStringLiteral("Fetching releases"), QString(), 0, 0,
                                              this, Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
        progress_dialog->setWindowTitle(QStringLiteral("Fetching releases"));

        connect(http_responses_future_watcher.get(),
                &QFutureWatcher<std::vector<std::shared_ptr<httplib::Response>>>::finished, this,
                [this] {
                    const std::vector<std::shared_ptr<httplib::Response>> responses =
                        http_responses_future_watcher->result();

                    http_responses_future_watcher.reset();
                    ui->releases_button->setEnabled(true);

                    progress_dialog.reset();

                    if (responses.empty()) {
                        QMessageBox::critical(this, QStringLiteral("Error"),
                                              QStringLiteral("Error while fetching the releases."));
                        return;
                    }

                    ui->releases->clear();

                    for (const std::shared_ptr<httplib::Response>& response : responses) {
                        if (!ui->releases->isEnabled()) {
                            ui->releases->setEnabled(true);
                            ui->releases_button->setText(QStringLiteral("Update"));
                        }

                        const nlohmann::json json = nlohmann::json::parse(response->body);
                        for (const nlohmann::json release : json) {
                            // Validate the version
                            if (semver::from_string_noexcept(release["tag_name"].get<std::string>())
                                    .has_value()) {
                                QListWidgetItem* item = new QListWidgetItem(
                                    QString::fromStdString(release["tag_name"]));

                                QStringList assets;
                                for (const nlohmann::json asset : release["assets"]) {
                                    assets
                                        << QString::fromStdString(asset["name"].get<std::string>());
                                }

                                item->setData(AssetsRole, assets);
                                ui->releases->addItem(item);
                            }
                        }
                    }
                });
        QFuture<std::vector<std::shared_ptr<httplib::Response>>> future = QtConcurrent::run([] {
            httplib::SSLClient client("api.github.com", 443);
            std::vector<std::shared_ptr<httplib::Response>> responses;
            const std::function<bool(const std::string&)> Next = [&Next, &client, &responses](
                                                                     const std::string& path) {
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
                                    const QString rel = match.captured(QStringViewLiteral("rel"));
                                    if (rel == QStringLiteral("next")) {
                                        const QString link =
                                            match.captured(QStringViewLiteral("link"));
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
        ui->releases_button->setEnabled(false);
        progress_dialog->exec();
    });

    Initialize();
    UpdateInstalledVersions();
}

ConfigureVersions::~ConfigureVersions() = default;
