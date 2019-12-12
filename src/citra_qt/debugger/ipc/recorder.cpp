// Copyright 2019 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QString>
#include <QTreeWidgetItem>
#include <fmt/format.h>
#include "citra_qt/debugger/ipc/record_dialog.h"
#include "citra_qt/debugger/ipc/recorder.h"
#include "common/assert.h"
#include "common/string_util.h"
#include "core/core.h"
#include "core/hle/kernel/ipc_debugger/recorder.h"
#include "core/hle/kernel/kernel.h"
#include "core/hle/service/sm/sm.h"
#include "ui_recorder.h"

IpcRecorderWidget::IpcRecorderWidget(QWidget* parent)
    : QDockWidget(parent), ui(std::make_unique<Ui::IPCRecorder>()) {

    ui->setupUi(this);
    qRegisterMetaType<IPCDebugger::RequestRecord>();

    connect(ui->enabled, &QCheckBox::stateChanged,
            [this](int new_state) { SetEnabled(new_state == Qt::Checked); });
    connect(ui->clearButton, &QPushButton::clicked, this, &IpcRecorderWidget::Clear);
    connect(ui->filter, &QLineEdit::textChanged, this, &IpcRecorderWidget::ApplyFilterToAll);
    connect(ui->main, &QTreeWidget::itemDoubleClicked, this, &IpcRecorderWidget::OpenRecordDialog);
    connect(this, &IpcRecorderWidget::EntryUpdated, this, &IpcRecorderWidget::OnEntryUpdated);
}

IpcRecorderWidget::~IpcRecorderWidget() = default;

void IpcRecorderWidget::OnEmulationStarting() {
    Clear();
    id_offset = 1;

    // Update the enabled status when the system is powered on.
    SetEnabled(ui->enabled->isChecked());
}

QString IpcRecorderWidget::GetStatusSQStringLiteral(
    const IPCDebugger::RequestRecord& record) const {
    switch (record.status) {
    case IPCDebugger::RequestStatus::Invalid:
        return QStringLiteral("Invalid");
    case IPCDebugger::RequestStatus::Sent:
        return QStringLiteral("Sent");
    case IPCDebugger::RequestStatus::Handling:
        return QStringLiteral("Handling");
    case IPCDebugger::RequestStatus::Handled:
        if (record.translated_reply_cmdbuf[1] == RESULT_SUCCESS.raw) {
            return QStringLiteral("Success");
        }
        return QStringLiteral("Error");
    case IPCDebugger::RequestStatus::HLEUnimplemented:
        return QStringLiteral("HLE Unimplemented");
    default:
        UNREACHABLE();
    }
}

void IpcRecorderWidget::OnEntryUpdated(IPCDebugger::RequestRecord record) {
    if (record.id < id_offset) { // The record has already been deleted by 'Clear'
        return;
    }

    QString service = GetServiceName(record);
    if (record.status == IPCDebugger::RequestStatus::Handling ||
        record.status == IPCDebugger::RequestStatus::Handled ||
        record.status == IPCDebugger::RequestStatus::HLEUnimplemented) {

        service = QStringLiteral("%1 (%2)").arg(service, record.is_hle ? QStringLiteral("HLE")
                                                                       : QStringLiteral("LLE"));
    }

    QTreeWidgetItem item{{QString::number(record.id), GetStatusSQStringLiteral(record), service,
                          GetFunctionName(record)}};

    const int row_id = record.id - id_offset;
    if (ui->main->invisibleRootItem()->childCount() > row_id) {
        records[row_id] = record;
        (*ui->main->invisibleRootItem()->child(row_id)) = item;
    } else {
        records.emplace_back(record);
        ui->main->invisibleRootItem()->addChild(new QTreeWidgetItem(item));
    }

    if (record.status == IPCDebugger::RequestStatus::HLEUnimplemented ||
        (record.status == IPCDebugger::RequestStatus::Handled &&
         record.translated_reply_cmdbuf[1] != RESULT_SUCCESS.raw)) { // Unimplemented / Error

        QTreeWidgetItem* item = ui->main->invisibleRootItem()->child(row_id);
        for (int column = 0; column < item->columnCount(); ++column) {
            item->setBackgroundColor(column, QColor::fromRgb(255, 0, 0));
        }
    }

    ApplyFilter(row_id);
}

void IpcRecorderWidget::SetEnabled(bool enabled) {
    if (!Core::System::GetInstance().IsPoweredOn()) {
        return;
    }

    IPCDebugger::Recorder& ipc_recorder = Core::System::GetInstance().Kernel().GetIPCRecorder();
    ipc_recorder.SetEnabled(enabled);

    if (enabled) {
        handle = ipc_recorder.BindCallback(
            [this](const IPCDebugger::RequestRecord& record) { emit EntryUpdated(record); });
    } else if (handle) {
        ipc_recorder.UnbindCallback(handle);
    }
}

void IpcRecorderWidget::Clear() {
    id_offset = records.size() + 1;

    records.clear();
    ui->main->invisibleRootItem()->takeChildren();
}

QString IpcRecorderWidget::GetServiceName(const IPCDebugger::RequestRecord& record) const {
    if (Core::System::GetInstance().IsPoweredOn() && record.client_port.id != -1) {
        const std::string service_name =
            Core::System::GetInstance().ServiceManager().GetServiceNameByPortId(
                static_cast<u32>(record.client_port.id));

        if (!service_name.empty()) {
            return QString::fromStdString(service_name);
        }
    }

    // Get a similar result from the server session name
    std::string session_name = record.server_session.name;
    session_name = Common::ReplaceAll(session_name, "_Server", "");
    session_name = Common::ReplaceAll(session_name, "_Client", "");
    return QString::fromStdString(session_name);
}

QString IpcRecorderWidget::GetFunctionName(const IPCDebugger::RequestRecord& record) const {
    if (record.untranslated_request_cmdbuf.empty()) { // Cmdbuf is not yet available
        return QStringLiteral("Unknown");
    }
    const QString header_code =
        QStringLiteral("0x%1").arg(record.untranslated_request_cmdbuf[0], 8, 16, QLatin1Char('0'));
    if (record.function_name.empty()) {
        return header_code;
    }
    return QStringLiteral("%1 (%2)").arg(QString::fromStdString(record.function_name), header_code);
}

void IpcRecorderWidget::ApplyFilter(int index) {
    QTreeWidgetItem* item = ui->main->invisibleRootItem()->child(index);
    const QString filter = ui->filter->text();
    if (filter.isEmpty()) {
        item->setHidden(false);
        return;
    }

    for (int i = 0; i < item->columnCount(); ++i) {
        if (item->text(i).contains(filter)) {
            item->setHidden(false);
            return;
        }
    }

    item->setHidden(true);
}

void IpcRecorderWidget::ApplyFilterToAll() {
    for (int i = 0; i < ui->main->invisibleRootItem()->childCount(); ++i) {
        ApplyFilter(i);
    }
}

void IpcRecorderWidget::OpenRecordDialog(QTreeWidgetItem* item, [[maybe_unused]] int column) {
    int index = ui->main->invisibleRootItem()->indexOfChild(item);

    RecordDialog dialog(this, records[static_cast<std::size_t>(index)], item->text(2),
                        item->text(3));
    dialog.exec();
}
