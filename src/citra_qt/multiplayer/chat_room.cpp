// Copyright 2017 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <array>
#include <future>
#include <QColor>
#include <QDesktopServices>
#include <QFutureWatcher>
#include <QImage>
#include <QList>
#include <QLocale>
#include <QMenu>
#include <QMessageBox>
#include <QMetaType>
#include <QTime>
#include <QUrl>
#include <QtConcurrent/QtConcurrentRun>
#include "citra_qt/game_list_p.h"
#include "citra_qt/multiplayer/chat_room.h"
#include "citra_qt/multiplayer/message.h"
#include "common/logging/log.h"
#include "core/announce_multiplayer_session.h"
#include "ui_chat_room.h"
#include "web_service/web_backend.h"

class ChatMessage {
public:
    explicit ChatMessage(const Network::ChatEntry& chat, QTime ts = {}) {
        /// Convert the time to their default locale defined format
        QLocale locale;
        timestamp = locale.toString(ts.isValid() ? ts : QTime::currentTime(), QLocale::ShortFormat);
        nickname = QString::fromStdString(chat.nickname);
        username = QString::fromStdString(chat.username);
        message = QString::fromStdString(chat.message);

        // Check for user pings
        QString cur_nickname, cur_username;
        if (const std::shared_ptr<Network::RoomMember> room_member =
                Network::GetRoomMember().lock()) {
            cur_nickname = QString::fromStdString(room_member->GetNickname());
            cur_username = QString::fromStdString(room_member->GetUsername());
        }

        // Handle pings at the beginning and end of message
        QString fixed_message = QStringLiteral(" %1 ").arg(message);
        if (fixed_message.contains(QStringLiteral(" @%1 ").arg(cur_nickname)) ||
            (!cur_username.isEmpty() &&
             fixed_message.contains(QStringLiteral(" @%1 ").arg(cur_username)))) {
            contains_ping = true;
        } else {
            contains_ping = false;
        }
    }

    bool ContainsPing() const {
        return contains_ping;
    }

    /// Format the message using the players color
    QString GetPlayerChatMessage(u16 player) const {
        const char* color = player_color[player % 16];
        QString name;
        if (username.isEmpty() || username == nickname) {
            name = nickname;
        } else {
            name = QStringLiteral("%1 (%2)").arg(nickname, username);
        }

        QString style, text_color;
        if (ContainsPing()) {
            // Add a background color to these messages
            style = QStringLiteral("background-color: %1").arg(ping_color);

            // Add a font color
            text_color = "color='#000000'";
        }

        return QStringLiteral("[%1] <font color='%2'>&lt;%3&gt;</font> <font style='%4' "
                              "%5>%6</font>")
            .arg(timestamp, color, name.toHtmlEscaped(), style, text_color,
                 message.toHtmlEscaped());
    }

private:
    static constexpr std::array<const char*, 16> player_color = {
        {"#0000FF", "#FF0000", "#8A2BE2", "#FF69B4", "#1E90FF", "#008000", "#00FF7F", "#B22222",
         "#DAA520", "#FF4500", "#2E8B57", "#5F9EA0", "#D2691E", "#9ACD32", "#FF7F50", "FFFF00"}};
    static constexpr char ping_color[] = "#FFFF00";

    QString timestamp;
    QString nickname;
    QString username;
    QString message;
    bool contains_ping;
};

class StatusMessage {
public:
    explicit StatusMessage(const QString& msg, QTime ts = {}) {
        /// Convert the time to their default locale defined format
        QLocale locale;
        timestamp = locale.toString(ts.isValid() ? ts : QTime::currentTime(), QLocale::ShortFormat);
        message = msg;
    }

    QString GetSystemChatMessage() const {
        return QStringLiteral("[%1] <font color='%2'>* %3</font>")
            .arg(timestamp, system_color, message);
    }

private:
    static constexpr const char system_color[] = "#FF8C00";
    QString timestamp;
    QString message;
};

class PlayerListItem : public QStandardItem {
public:
    static const int NicknameRole = Qt::UserRole + 1;
    static const int UsernameRole = Qt::UserRole + 2;
    static const int AvatarUrlRole = Qt::UserRole + 3;
    static const int GameNameRole = Qt::UserRole + 4;

    PlayerListItem() = default;
    explicit PlayerListItem(const std::string& nickname, const std::string& username,
                            const std::string& avatar_url, const std::string& game_name) {
        setEditable(false);
        setData(QString::fromStdString(nickname), NicknameRole);
        setData(QString::fromStdString(username), UsernameRole);
        setData(QString::fromStdString(avatar_url), AvatarUrlRole);
        if (game_name.empty()) {
            setData(QStringLiteral("Not playing a game"), GameNameRole);
        } else {
            setData(QString::fromStdString(game_name), GameNameRole);
        }
    }

    QVariant data(int role) const override {
        if (role != Qt::DisplayRole) {
            return QStandardItem::data(role);
        }
        QString name;
        const QString nickname = data(NicknameRole).toString();
        const QString username = data(UsernameRole).toString();
        if (username.isEmpty() || username == nickname) {
            name = nickname;
        } else {
            name = QStringLiteral("%1 (%2)").arg(nickname, username);
        }
        return QStringLiteral("%1\n      %2").arg(name, data(GameNameRole).toString());
    }
};

ChatRoom::ChatRoom(QWidget* parent) : QWidget(parent), ui(std::make_unique<Ui::ChatRoom>()) {
    ui->setupUi(this);

    // Set the item_model for member_view

    members = new QStandardItemModel(ui->member_view);
    ui->member_view->setModel(members);
    ui->member_view->setContextMenuPolicy(Qt::CustomContextMenu);

    // Set a header to make it look better though there is only one column
    members->insertColumns(0, 1);
    members->setHeaderData(0, Qt::Horizontal, QStringLiteral("Members"));

    ui->chat_history->document()->setMaximumBlockCount(max_chat_lines);

    // Register the network structs to use in slots and signals
    qRegisterMetaType<Network::ChatEntry>();
    qRegisterMetaType<Network::StatusMessageEntry>();
    qRegisterMetaType<Network::RoomInformation>();
    qRegisterMetaType<Network::RoomMember::State>();

    // Setup the callbacks for network updates
    if (std::shared_ptr<Network::RoomMember> room_member = Network::GetRoomMember().lock()) {
        room_member->BindOnChatMessageRecieved(
            [this](const Network::ChatEntry& chat) { emit ChatReceived(chat); });
        room_member->BindOnStatusMessageReceived(
            [this](const Network::StatusMessageEntry& status_message) {
                emit StatusMessageReceived(status_message);
            });
        connect(this, &ChatRoom::ChatReceived, this, &ChatRoom::OnChatReceive);
        connect(this, &ChatRoom::StatusMessageReceived, this, &ChatRoom::OnStatusMessageReceive);
    } else {
        // TODO: do something if network was not initialized
    }

    // Connect all the widgets to the appropriate events
    connect(ui->member_view, &QTreeView::customContextMenuRequested, this,
            &ChatRoom::PopupContextMenu);
    connect(ui->chat_message, &QLineEdit::returnPressed, this, &ChatRoom::OnSendChat);
    connect(ui->chat_message, &QLineEdit::textChanged, this, &ChatRoom::OnChatTextChanged);
    connect(ui->send_message, &QPushButton::clicked, this, &ChatRoom::OnSendChat);
}

ChatRoom::~ChatRoom() = default;

void ChatRoom::SetModPerms(bool is_mod) {
    has_mod_perms = is_mod;
}

void ChatRoom::Clear() {
    ui->chat_history->clear();
    block_list.clear();
}

void ChatRoom::AppendStatusMessage(const QString& msg) {
    ui->chat_history->append(StatusMessage(msg).GetSystemChatMessage());
}

void ChatRoom::AppendChatMessage(const QString& msg) {
    ui->chat_history->append(msg);
}

void ChatRoom::SendModerationRequest(Network::RoomMessageTypes type, const std::string& nickname) {
    if (std::shared_ptr<Network::RoomMember> room_member = Network::GetRoomMember().lock()) {
        const Network::RoomMember::MemberList members = room_member->GetMemberInformation();
        std::vector<Network::RoomMember::MemberInformation>::const_iterator it =
            std::find_if(members.begin(), members.end(),
                         [&nickname](const Network::RoomMember::MemberInformation& member) {
                             return member.nickname == nickname;
                         });
        if (it == members.end()) {
            NetworkMessage::ShowError(NetworkMessage::NO_SUCH_USER);
            return;
        }
        room_member->SendModerationRequest(type, nickname);
    }
}

bool ChatRoom::ValidateMessage(const std::string& msg) {
    return !msg.empty();
}

void ChatRoom::OnRoomUpdate(const Network::RoomInformation& info) {
    // TODO(B3N30): change title
    if (std::shared_ptr<Network::RoomMember> room_member = Network::GetRoomMember().lock()) {
        SetPlayerList(room_member->GetMemberInformation());
    }
}

void ChatRoom::Disable() {
    ui->send_message->setDisabled(true);
    ui->chat_message->setDisabled(true);
}

void ChatRoom::Enable() {
    ui->send_message->setEnabled(true);
    ui->chat_message->setEnabled(true);
}

void ChatRoom::OnChatReceive(const Network::ChatEntry& chat) {
    if (!ValidateMessage(chat.message)) {
        return;
    }
    if (const std::shared_ptr<Network::RoomMember> room_member = Network::GetRoomMember().lock()) {
        // Get the id of the player
        const Network::RoomMember::MemberList& members = room_member->GetMemberInformation();
        std::vector<Network::RoomMember::MemberInformation>::const_iterator it = std::find_if(
            members.begin(), members.end(),
            [&chat](const Network::RoomMember::MemberInformation& member) {
                return member.nickname == chat.nickname && member.username == chat.username;
            });
        if (it == members.end()) {
            LOG_INFO(Network, "Chat message received from unknown player. Ignoring it.");
            return;
        }
        if (block_list.count(chat.nickname)) {
            LOG_INFO(Network, "Chat message received from blocked player {}. Ignoring it.",
                     chat.nickname);
            return;
        }
        std::ptrdiff_t player = std::distance(members.begin(), it);
        ChatMessage m(chat);
        if (m.ContainsPing()) {
            emit UserPinged();
        }
        AppendChatMessage(m.GetPlayerChatMessage(player));
    }
}

void ChatRoom::OnStatusMessageReceive(const Network::StatusMessageEntry& status_message) {
    QString name;
    if (status_message.username.empty() || status_message.username == status_message.nickname) {
        name = QString::fromStdString(status_message.nickname);
    } else {
        name = QStringLiteral("%1 (%2)").arg(QString::fromStdString(status_message.nickname),
                                             QString::fromStdString(status_message.username));
    }
    QString message;
    switch (status_message.type) {
    case Network::IdMemberJoin:
        message = QStringLiteral("%1 has joined").arg(name);
        break;
    case Network::IdMemberLeave:
        message = QStringLiteral("%1 has left").arg(name);
        break;
    case Network::IdMemberKicked:
        message = QStringLiteral("%1 has been kicked").arg(name);
        break;
    case Network::IdMemberBanned:
        message = QStringLiteral("%1 has been banned").arg(name);
        break;
    case Network::IdAddressUnbanned:
        message = QStringLiteral("%1 has been unbanned").arg(name);
        break;
    }
    if (!message.isEmpty()) {
        AppendStatusMessage(message);
    }
}

void ChatRoom::OnSendChat() {
    if (std::shared_ptr<Network::RoomMember> room_member = Network::GetRoomMember().lock()) {
        if (room_member->GetState() != Network::RoomMember::State::Joined &&
            room_member->GetState() != Network::RoomMember::State::Moderator) {
            return;
        }
        const std::string message = ui->chat_message->text().toStdString();
        if (!ValidateMessage(message)) {
            return;
        }
        const std::string nick = room_member->GetNickname();
        const std::string username = room_member->GetUsername();
        const Network::ChatEntry chat{nick, username, message};

        std::vector<Network::RoomMember::MemberInformation> members =
            room_member->GetMemberInformation();
        std::vector<Network::RoomMember::MemberInformation>::iterator it = std::find_if(
            members.begin(), members.end(),
            [&chat](const Network::RoomMember::MemberInformation& member) {
                return member.nickname == chat.nickname && member.username == chat.username;
            });
        if (it == members.end()) {
            LOG_INFO(Network, "Cannot find self in the player list when sending a message.");
        }
        const std::ptrdiff_t player = std::distance(members.begin(), it);
        const ChatMessage m(chat);
        room_member->SendChatMessage(message);
        AppendChatMessage(m.GetPlayerChatMessage(player));
        ui->chat_message->clear();
    }
}

void ChatRoom::UpdateIconDisplay() {
    for (int row = 0; row < members->invisibleRootItem()->rowCount(); ++row) {
        QStandardItem* item = members->invisibleRootItem()->child(row);
        const std::string avatar_url =
            item->data(PlayerListItem::AvatarUrlRole).toString().toStdString();
        if (icon_cache.count(avatar_url)) {
            item->setData(icon_cache.at(avatar_url), Qt::DecorationRole);
        } else {
            item->setData(QIcon::fromTheme("no_avatar").pixmap(48), Qt::DecorationRole);
        }
    }
}

void ChatRoom::SetPlayerList(const Network::RoomMember::MemberList& members) {
    // TODO(B3N30): Remember which row is selected
    this->members->removeRows(0, this->members->rowCount());
    for (const Network::RoomMember::MemberInformation& member : members) {
        if (member.nickname.empty()) {
            continue;
        }
        QStandardItem* name_item = new PlayerListItem(member.nickname, member.username,
                                                      member.avatar_url, member.game_info.name);

        if (!icon_cache.count(member.avatar_url) && !member.avatar_url.empty()) {
            // Start a request to get the member's avatar
            const QUrl url(QString::fromStdString(member.avatar_url));
            QFuture<std::string> future = QtConcurrent::run([url] {
                WebService::Client client(
                    QStringLiteral("%1://%2").arg(url.scheme(), url.host()).toStdString(), "", "");
                const Common::WebResult result = client.GetImage(url.path().toStdString(), true);
                if (result.returned_data.empty()) {
                    LOG_ERROR(WebService, "Failed to get avatar");
                }
                return result.returned_data;
            });
            QFutureWatcher<std::string>* future_watcher = new QFutureWatcher<std::string>(this);
            connect(future_watcher, &QFutureWatcher<std::string>::finished, this,
                    [this, future_watcher, avatar_url = member.avatar_url] {
                        const std::string result = future_watcher->result();
                        if (result.empty()) {
                            return;
                        }
                        QPixmap pixmap;
                        if (!pixmap.loadFromData(reinterpret_cast<const u8*>(result.data()),
                                                 static_cast<uint>(result.size()))) {
                            return;
                        }
                        icon_cache[avatar_url] =
                            pixmap.scaled(48, 48, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                        // Update all the displayed icons with the new icon_cache
                        UpdateIconDisplay();
                    });
            future_watcher->setFuture(future);
        }

        this->members->invisibleRootItem()->appendRow(name_item);
    }
    UpdateIconDisplay();
    // TODO(B3N30): Restore row selection
}

void ChatRoom::OnChatTextChanged() {
    if (ui->chat_message->text().length() > static_cast<int>(Network::MaxMessageSize)) {
        ui->chat_message->setText(
            ui->chat_message->text().left(static_cast<int>(Network::MaxMessageSize)));
    }
}

void ChatRoom::PopupContextMenu(const QPoint& menu_location) {
    QModelIndex item = ui->member_view->indexAt(menu_location);
    if (!item.isValid()) {
        return;
    }

    std::string nickname =
        members->item(item.row())->data(PlayerListItem::NicknameRole).toString().toStdString();

    QMenu context_menu;

    QString username = members->item(item.row())->data(PlayerListItem::UsernameRole).toString();
    if (!username.isEmpty()) {
        QAction* view_profile_action = context_menu.addAction(QStringLiteral("View Profile"));
        connect(view_profile_action, &QAction::triggered, [username] {
            QDesktopServices::openUrl(
                QUrl(QStringLiteral("https://community.citra-emu.org/u/%1").arg(username)));
        });
    }

    std::string cur_nickname;
    if (const std::shared_ptr<Network::RoomMember> room_member = Network::GetRoomMember().lock()) {
        cur_nickname = room_member->GetNickname();
    }

    if (nickname != cur_nickname) { // You can't block yourself
        QAction* block_action =
            context_menu.addAction(QStringLiteral("Block Player"), [this, nickname] {
                if (block_list.count(nickname)) {
                    block_list.erase(nickname);
                } else if (QMessageBox::question(
                               this, QStringLiteral("Block Player"),
                               QStringLiteral(
                                   "When you block a player, you will no longer receive chat "
                                   "messages from "
                                   "them.<br><br>Are you sure you would like to block %1?")
                                   .arg(QString::fromStdString(nickname)),
                               QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
                    block_list.emplace(nickname);
                }
            });
        block_action->setCheckable(true);
        block_action->setChecked(block_list.count(nickname) > 0);
    }

    if (has_mod_perms && nickname != cur_nickname) { // You can't kick or ban yourself
        context_menu.addSeparator();

        context_menu.addAction(QStringLiteral("Kick"), [this, nickname] {
            if (QMessageBox::question(
                    this, QStringLiteral("Kick Player"),
                    QStringLiteral("Are you sure you would like to <b>kick</b> %1?")
                        .arg(QString::fromStdString(nickname)),
                    QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
                SendModerationRequest(Network::IdModKick, nickname);
            }
        });

        context_menu.addAction(QStringLiteral("Ban"), [this, nickname] {
            if (QMessageBox::question(
                    this, QStringLiteral("Ban Player"),
                    QStringLiteral(
                        "Are you sure you would like to <b>kick and ban</b> %1?\n\nThis would "
                        "ban both their forum username and their IP address.")
                        .arg(QString::fromStdString(nickname)),
                    QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
                SendModerationRequest(Network::IdModBan, nickname);
            }
        });
    }

    context_menu.exec(ui->member_view->viewport()->mapToGlobal(menu_location));
}
