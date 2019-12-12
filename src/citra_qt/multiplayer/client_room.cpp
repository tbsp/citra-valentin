// Copyright 2017 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <future>
#include <QColor>
#include <QImage>
#include <QList>
#include <QLocale>
#include <QMetaType>
#include <QTime>
#include <QtConcurrent/QtConcurrentRun>
#include "citra_qt/game_list_p.h"
#include "citra_qt/multiplayer/client_room.h"
#include "citra_qt/multiplayer/message.h"
#include "citra_qt/multiplayer/moderation_dialog.h"
#include "citra_qt/multiplayer/state.h"
#include "common/logging/log.h"
#include "core/announce_multiplayer_session.h"
#include "ui_client_room.h"

ClientRoomWindow::ClientRoomWindow(QWidget* parent)
    : QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowSystemMenuHint),
      ui(std::make_unique<Ui::ClientRoom>()) {
    ui->setupUi(this);

    // Setup the callbacks for network updates
    if (std::shared_ptr<Network::RoomMember> room_member = Network::GetRoomMember().lock()) {
        room_member->BindOnRoomInformationChanged(
            [this](const Network::RoomInformation& /*info*/) { emit RoomInformationChanged(); });
        room_member->BindOnStateChanged(
            [this](const Network::RoomMember::State& state) { emit StateChanged(state); });

        connect(this, &ClientRoomWindow::RoomInformationChanged, this,
                &ClientRoomWindow::OnRoomUpdate);
        connect(this, &ClientRoomWindow::StateChanged, this, &::ClientRoomWindow::OnStateChange);
        // Update the state
        OnStateChange(room_member->GetState());
    } else {
        // TODO: do something if network was not initialized
    }

    connect(ui->disconnect, &QPushButton::clicked, this, &ClientRoomWindow::Disconnect);
    ui->disconnect->setDefault(false);
    ui->disconnect->setAutoDefault(false);
    connect(ui->moderation, &QPushButton::clicked, [this] {
        ModerationDialog dialog(this);
        dialog.exec();
    });
    ui->moderation->setDefault(false);
    ui->moderation->setAutoDefault(false);
    connect(ui->chat, &ChatRoom::UserPinged, this, &ClientRoomWindow::ShowNotification);
    UpdateView();
}

ClientRoomWindow::~ClientRoomWindow() = default;

void ClientRoomWindow::SetModPerms(bool is_mod) {
    ui->chat->SetModPerms(is_mod);
    ui->moderation->setVisible(is_mod);
    ui->moderation->setDefault(false);
    ui->moderation->setAutoDefault(false);
}

void ClientRoomWindow::OnRoomUpdate() {
    UpdateView();
}

void ClientRoomWindow::OnStateChange(const Network::RoomMember::State& state) {
    if (state == Network::RoomMember::State::Joined ||
        state == Network::RoomMember::State::Moderator) {

        ui->chat->Clear();
        ui->chat->AppendStatusMessage(QStringLiteral("Connected"));
        SetModPerms(state == Network::RoomMember::State::Moderator);
    }
    UpdateView();
}

void ClientRoomWindow::Disconnect() {
    MultiplayerState* parent = static_cast<MultiplayerState*>(parentWidget());
    if (parent->OnCloseRoom()) {
        ui->chat->AppendStatusMessage(QStringLiteral("Disconnected"));
        close();
    }
}

void ClientRoomWindow::UpdateView() {
    if (std::shared_ptr<Network::RoomMember> room_member = Network::GetRoomMember().lock()) {
        if (room_member->IsConnected()) {
            ui->chat->Enable();
            ui->disconnect->setEnabled(true);
            const std::vector<Network::RoomMember::MemberInformation> members =
                room_member->GetMemberInformation();
            ui->chat->SetPlayerList(members);
            const Network::RoomInformation information = room_member->GetRoomInformation();
            setWindowTitle(QString(QStringLiteral("%1 (%2/%3 members) - connected"))
                               .arg(QString::fromStdString(information.name))
                               .arg(members.size())
                               .arg(information.member_slots));
            ui->description->setText(QString::fromStdString(information.description));
            return;
        }
    }
    // TODO: can't get RoomMember, show error and close window
    close();
}

void ClientRoomWindow::UpdateIconDisplay() {
    ui->chat->UpdateIconDisplay();
}
