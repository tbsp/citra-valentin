// Copyright 2019 Citra Valentin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QApplication>
#include <QString>
#include <QThread>
#include <QTimer>
#include "citra_qt/debugger/profiler.h"
#include "common/common_types.h"
#include "common/logging/log.h"
#include "ui_profiler.h"

enum class Column {
    Category,
    Scope,
    Current,
    Minimum,
    Maximum,
};

QtProfiler::QtProfiler(QWidget* parent)
    : QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint),
      ui(std::make_unique<Ui::Profiler>()) {
    ui->setupUi(this);
    ui->table->setColumnWidth(static_cast<int>(Column::Scope), 150);
}

QtProfiler::~QtProfiler() = default;

void QtProfiler::Set(const std::string category, const std::string scope,
                     const long long milliseconds) {
    std::lock_guard<std::mutex> lock(mutex);

    auto it = current.find(category + scope);
    if (it == current.end()) {
        current.insert(
            {category + scope, {-1, category, scope, milliseconds, milliseconds, milliseconds}});
    } else {
        it->second.current = milliseconds;

        if (milliseconds < it->second.minimum) {
            it->second.minimum = milliseconds;
        } else if (milliseconds > it->second.maximum) {
            it->second.maximum = milliseconds;
        }
    }
}

void QtProfiler::Update() {
    if (QThread::currentThread() != qApp->thread()) {
        QMetaObject::invokeMethod(this, "Update", Qt::BlockingQueuedConnection);
        return;
    }

    std::lock_guard<std::mutex> lock(mutex);

    if (stopping) {
        return;
    }

    if (stop_requested) {
        emit Stopped();
        stopping = true;
        return;
    }

    for (std::pair<const std::string, QtProfiler::ScopeData>& scope_data : current) {
        if (scope_data.second.row == -1) {
            const int row = ui->table->rowCount();
            ui->table->insertRow(row);
            ui->table->setItem(
                row, 0, new QTableWidgetItem(QString::fromStdString(scope_data.second.category)));
            ui->table->setItem(
                row, 1, new QTableWidgetItem(QString::fromStdString(scope_data.second.scope)));
            scope_data.second.row = row;
        }

        ui->table->setItem(
            scope_data.second.row, 2,
            new QTableWidgetItem(
                QStringLiteral("%1 milliseconds").arg(QString::number(scope_data.second.current))));

        ui->table->setItem(
            scope_data.second.row, 3,
            new QTableWidgetItem(
                QStringLiteral("%1 milliseconds").arg(QString::number(scope_data.second.minimum))));

        ui->table->setItem(
            scope_data.second.row, 4,
            new QTableWidgetItem(
                QStringLiteral("%1 milliseconds").arg(QString::number(scope_data.second.maximum))));
    }
}

void QtProfiler::Stop() {
    std::lock_guard<std::mutex> lock(mutex);
    stop_requested = true;
}
