// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QDesktopServices>
#include <QDir>
#include <QDirIterator>
#include <QMenu>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QUrl>
#include "citra_qt/configuration/configure_versions.h"
#include "common/logging/log.h"
#include "common/version.h"
#include "ui_configure_versions.h"

const QRegularExpression DIR_REGEX(
    QStringLiteral("citra-valentin-windows-(?P<version>(?P<major>0|[1-9]\\d*)\\.(?P<minor>0|[1-9]"
                   "\\d*)\\.(?P<patch>0|[1-9]\\d*)(?:-(?P<prerelease>(?:0|[1-9]\\d*|\\d*[a-zA-Z-]["
                   "0-9a-zA-Z-]*)(?:\\.(?:0|[1-9]\\d*|\\d*[a-zA-Z-][0-9a-zA-Z-]*))*))?(?:\\+(?P<"
                   "buildmetadata>[0-9a-zA-Z-]+(?:\\.[0-9a-zA-Z-]+)*))?)"));

void ConfigureVersions::Initialize() {
    ui->versions_installed_using_cvu_versions_group->setEnabled(true);

    // Set the installation directory
    char* cvu_install_dir = getenv("CVU_INSTALL_DIR");
    install_dir = cvu_install_dir == nullptr
                      ? QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) +
                            "/citra-valentin"
                      : QString::fromLocal8Bit(cvu_install_dir);

    connect(ui->update_versions_installed_using_cvu, &QPushButton::clicked, this,
            &ConfigureVersions::UpdateInstalledVersions);

    connect(
        ui->versions_installed_using_cvu_versions_group, &QListWidget::customContextMenuRequested,
        this, [this](const QPoint& position) {
            QListWidgetItem* item = ui->versions_installed_using_cvu->currentItem();
            if (item == nullptr) {
                return;
            }

            const semver::version version = item->data(VersionRole).value<semver::version>();
            QMenu menu(QString::fromStdString(version.to_string()), this);

            QAction action_start(QStringLiteral("Start"), this);
            connect(&action_start, &QAction::triggered, this, [this, item] {
                QProcess* process = new QProcess(this);
                process->startDetached(
                    QStringLiteral("%1/citra-valentin-qt.exe").arg(item->data(PathRole).toString()),
                    QStringList());
            });
            menu.addAction(&action_start);

            QAction action_open_changelog(QStringLiteral("Open Changelog"), this);
            // There's no changelog for versions before 2.10.1 currently.
            if (version >= semver::version{2, 10, 1}) {
                connect(&action_open_changelog, &QAction::triggered, this, [this, &version] {
                    QDesktopServices::openUrl(
                        QUrl(QStringLiteral("https://github.com/vvanelslande/citra/blob/"
                                            "master/changelog.md#%1")
                                 .arg(QString::fromStdString(version.to_string())
                                          .remove(QLatin1Char{'.'}))));
                });

                menu.addAction(&action_open_changelog);
            }

            QAction action_open_location(QStringLiteral("Open Location"), this);
            connect(&action_open_location, &QAction::triggered, this, [this, item] {
                QDesktopServices::openUrl(QUrl::fromLocalFile(item->data(PathRole).toString()));
            });

            menu.addAction(&action_open_location);

            QAction action_delete(QStringLiteral("Delete"), this);
            if (version != Version::citra_valentin) {
                connect(&action_delete, &QAction::triggered, this, [this, item] {
                    QDir dir(item->data(PathRole).toString());
                    dir.removeRecursively();
                    delete item;
                });
                menu.addAction(&action_delete);
            }

            menu.exec(mapToGlobal(position));
        });
}

void ConfigureVersions::UpdateInstalledVersions() {
    ui->versions_installed_using_cvu->clear();

    QDirIterator it(install_dir, QDir::Dirs | QDir::NoDotAndDotDot);

    while (it.hasNext()) {
        const QString path = it.next();
        QDir dir(path);

        // Get the version from the dir name
        const QString dir_name = dir.dirName();

        const QRegularExpressionMatch match = DIR_REGEX.match(dir_name);
        if (match.hasMatch()) {
            // CVU adds -disabled to the folder name.
            const semver::version version(match.captured(QStringViewLiteral("version"))
                                              .remove(QStringLiteral("-disabled"))
                                              .toStdString());

            QListWidgetItem* item = new QListWidgetItem(
                ((version == Version::citra_valentin) ? QStringLiteral("%1 (Running)")
                                                      : QStringLiteral("%1"))
                    .arg(QString::fromStdString(version.to_string())));

            item->setData(VersionRole, QVariant::fromValue<semver::version>(version));
            item->setData(PathRole, path);

            ui->versions_installed_using_cvu->addItem(item);
        } else {
            LOG_WARNING(Frontend, "Ignoring directory {}", dir_name.toStdString());
        }
    }
}