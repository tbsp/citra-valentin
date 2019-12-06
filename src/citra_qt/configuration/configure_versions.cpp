// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QDesktopServices>
#include <QDialog>
#include <QFutureWatcher>
#include <QLabel>
#include <QProgressBar>
#include <QUrl>
#include <QVBoxLayout>
#include <QtConcurrent/QtConcurrentRun>
#include <json.hpp>
#include "citra_qt/configuration/configure_versions.h"
#include "common/logging/log.h"
#include "ui_configure_versions.h"
#include "web_service/web_backend.h"

ConfigureVersions::ConfigureVersions(QWidget* parent)
    : QWidget(parent), ui(std::make_unique<Ui::ConfigureVersions>()) {
    ui->setupUi(this);

    ui->versions_installed_using_cvu_group->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->recent_releases->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->recent_releases, &QListWidget::doubleClicked, this, [this] {
        QListWidgetItem* item = ui->recent_releases->currentItem();
        if (item == nullptr) {
            return;
        }

        QDesktopServices::openUrl(
            QUrl(QStringLiteral("https://github.com/vvanelslande/citra/releases/tag/%1")
                     .arg(item->text())));
    });

    connect(ui->recent_releases_button, &QPushButton::clicked, this, [this] {
        fetch_recent_releases_watcher = std::make_unique<QFutureWatcher<Common::WebResult>>();
        fetching_recent_releases_dialog =
            std::make_unique<QDialog>(this, Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
        fetching_recent_releases_dialog->setWindowTitle(QStringLiteral("Fetching recent releases"));
        fetching_recent_releases_dialog->setFixedWidth(250);
        QVBoxLayout* layout = new QVBoxLayout(fetching_recent_releases_dialog.get());
        layout->addWidget(new QLabel(QStringLiteral("Fetching recent releases..."),
                                     fetching_recent_releases_dialog.get()));
        QProgressBar* progress_bar = new QProgressBar(fetching_recent_releases_dialog.get());
        progress_bar->setMinimum(0);
        progress_bar->setMaximum(0);
        layout->addWidget(progress_bar);

        connect(fetch_recent_releases_watcher.get(), &QFutureWatcher<Common::WebResult>::finished,
                this, [this] {
                    const Common::WebResult result = fetch_recent_releases_watcher->result();

                    fetch_recent_releases_watcher.reset();
                    ui->recent_releases_button->setEnabled(true);

                    fetching_recent_releases_dialog.reset();

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
        fetch_recent_releases_watcher->setFuture(future);
        ui->recent_releases_button->setEnabled(false);
        fetching_recent_releases_dialog->exec();
    });

    Initialize();
    UpdateInstalledVersions();
}

ConfigureVersions::~ConfigureVersions() = default;
