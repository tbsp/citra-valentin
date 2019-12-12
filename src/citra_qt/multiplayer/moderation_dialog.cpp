// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QStandardItem>
#include <QStandardItemModel>
#include "citra_qt/multiplayer/moderation_dialog.h"
#include "network/network.h"
#include "network/room_member.h"
#include "ui_moderation_dialog.h"

namespace Column {
enum {
    SUBJECT,
    TYPE,
    COUNT,
};
}

ModerationDialog::ModerationDialog(QWidget* parent)
    : QDialog(parent), ui(std::make_unique<Ui::ModerationDialog>()) {
    ui->setupUi(this);

    qRegisterMetaType<Network::Room::BanList>();

    if (std::shared_ptr<Network::RoomMember> room_member = Network::GetRoomMember().lock()) {
        callback_handle_status_message = room_member->BindOnStatusMessageReceived(
            [this](const Network::StatusMessageEntry& status_message) {
                emit StatusMessageReceived(status_message);
            });
        connect(this, &ModerationDialog::StatusMessageReceived, this,
                &ModerationDialog::OnStatusMessageReceived);
        callback_handle_ban_list = room_member->BindOnBanListReceived(
            [this](const Network::Room::BanList& ban_list) { emit BanListReceived(ban_list); });
        connect(this, &ModerationDialog::BanListReceived, this, &ModerationDialog::PopulateBanList);
    }

    // Initialize the UI
    model = new QStandardItemModel(ui->ban_list_view);
    model->insertColumns(0, Column::COUNT);
    model->setHeaderData(Column::SUBJECT, Qt::Horizontal, QStringLiteral("Subject"));
    model->setHeaderData(Column::TYPE, Qt::Horizontal, QStringLiteral("Type"));

    ui->ban_list_view->setModel(model);

    // Load the ban list in background
    LoadBanList();

    connect(ui->refresh, &QPushButton::clicked, this, [this] { LoadBanList(); });
    connect(ui->unban, &QPushButton::clicked, this, [this] {
        const QModelIndex index = ui->ban_list_view->currentIndex();
        SendUnbanRequest(model->item(index.row(), 0)->text());
    });
    connect(ui->ban_list_view, &QTreeView::clicked, [this] { ui->unban->setEnabled(true); });
}

ModerationDialog::~ModerationDialog() {
    if (std::shared_ptr<Network::RoomMember> room_member = Network::GetRoomMember().lock()) {
        if (callback_handle_status_message) {
            room_member->Unbind(callback_handle_status_message);
        }

        if (callback_handle_ban_list) {
            room_member->Unbind(callback_handle_ban_list);
        }
    }
}

void ModerationDialog::LoadBanList() {
    if (std::shared_ptr<Network::RoomMember> room_member = Network::GetRoomMember().lock()) {
        ui->refresh->setEnabled(false);
        ui->refresh->setText(QStringLiteral("Refreshing"));
        ui->unban->setEnabled(false);
        room_member->RequestBanList();
    }
}

void ModerationDialog::PopulateBanList(const Network::Room::BanList& ban_list) {
    model->removeRows(0, model->rowCount());
    for (const std::string& username : ban_list.first) {
        QStandardItem* subject_item = new QStandardItem(QString::fromStdString(username));
        QStandardItem* type_item = new QStandardItem(QStringLiteral("Forum Username"));
        model->invisibleRootItem()->appendRow({subject_item, type_item});
    }
    for (const std::string& ip : ban_list.second) {
        QStandardItem* subject_item = new QStandardItem(QString::fromStdString(ip));
        QStandardItem* type_item = new QStandardItem(QStringLiteral("IP Address"));
        model->invisibleRootItem()->appendRow({subject_item, type_item});
    }
    for (int i = 0; i < Column::COUNT - 1; ++i) {
        ui->ban_list_view->resizeColumnToContents(i);
    }
    ui->refresh->setEnabled(true);
    ui->refresh->setText(QStringLiteral("Refresh"));
    ui->unban->setEnabled(false);
}

void ModerationDialog::SendUnbanRequest(const QString& subject) {
    if (std::shared_ptr<Network::RoomMember> room_member = Network::GetRoomMember().lock()) {
        room_member->SendModerationRequest(Network::IdModUnban, subject.toStdString());
    }
}

void ModerationDialog::OnStatusMessageReceived(const Network::StatusMessageEntry& status_message) {
    if (status_message.type != Network::IdMemberBanned &&
        status_message.type != Network::IdAddressUnbanned)
        return;

    // Update the ban list for ban/unban
    LoadBanList();
}
