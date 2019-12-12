// Copyright 2017 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QInputDialog>
#include <QList>
#include <QtConcurrent/QtConcurrentRun>
#include "citra_qt/game_list_p.h"
#include "citra_qt/main.h"
#include "citra_qt/multiplayer/client_room.h"
#include "citra_qt/multiplayer/lobby.h"
#include "citra_qt/multiplayer/lobby_p.h"
#include "citra_qt/multiplayer/message.h"
#include "citra_qt/multiplayer/state.h"
#include "citra_qt/multiplayer/validation.h"
#include "citra_qt/uisettings.h"
#include "common/logging/log.h"
#include "core/hle/service/cfg/cfg.h"
#include "core/settings.h"
#include "network/network.h"
#include "web_service/web_backend.h"

Lobby::Lobby(QWidget* parent, QStandardItemModel* list,
             std::shared_ptr<Core::AnnounceMultiplayerSession> session)
    : QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowSystemMenuHint),
      ui(std::make_unique<Ui::Lobby>()), announce_multiplayer_session(session) {
    ui->setupUi(this);

    // setup the watcher for background connections
    watcher = new QFutureWatcher<void>;

    model = new QStandardItemModel(ui->room_list);

    // Create a proxy to the game list to get the list of games owned
    game_list = new QStandardItemModel;
    UpdateGameList(list);

    proxy = new LobbyFilterProxyModel(this, game_list);
    proxy->setSourceModel(model);
    proxy->setDynamicSortFilter(true);
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxy->setSortLocaleAware(true);
    ui->room_list->setModel(proxy);
    ui->room_list->header()->setSectionResizeMode(QHeaderView::Interactive);
    ui->room_list->header()->stretchLastSection();
    ui->room_list->setAlternatingRowColors(true);
    ui->room_list->setSelectionMode(QHeaderView::SingleSelection);
    ui->room_list->setSelectionBehavior(QHeaderView::SelectRows);
    ui->room_list->setVerticalScrollMode(QHeaderView::ScrollPerPixel);
    ui->room_list->setHorizontalScrollMode(QHeaderView::ScrollPerPixel);
    ui->room_list->setSortingEnabled(true);
    ui->room_list->setEditTriggers(QHeaderView::NoEditTriggers);
    ui->room_list->setExpandsOnDoubleClick(false);
    ui->room_list->setContextMenuPolicy(Qt::CustomContextMenu);

    ui->nickname->setValidator(validation.GetNickname());
    ui->nickname->setText(UISettings::values.nickname);
    if (ui->nickname->text().isEmpty() && !Settings::values.citra_username.empty()) {
        // Use Citra Web Service user name as nickname by default
        ui->nickname->setText(QString::fromStdString(Settings::values.citra_username));
    }

    // UI Buttons
    connect(ui->refresh_list, &QPushButton::clicked, this, &Lobby::RefreshLobby);
    connect(ui->games_owned, &QCheckBox::toggled, proxy, &LobbyFilterProxyModel::SetFilterOwned);
    connect(ui->hide_full, &QCheckBox::toggled, proxy, &LobbyFilterProxyModel::SetFilterFull);
    connect(ui->search, &QLineEdit::textChanged, proxy, &LobbyFilterProxyModel::SetFilterSearch);
    connect(ui->room_list, &QTreeView::doubleClicked, this, &Lobby::OnJoinRoom);
    connect(ui->room_list, &QTreeView::clicked, this, &Lobby::OnExpandRoom);

    // Actions
    connect(&room_list_watcher, &QFutureWatcher<AnnounceMultiplayerRoom::RoomList>::finished, this,
            &Lobby::OnRefreshLobby);

    // manually start a refresh when the window is opening
    // TODO(jroweboy): if this refresh is slow for people with bad internet, then don't do it as
    // part of the constructor, but offload the refresh until after the window shown. perhaps emit a
    // refreshroomlist signal from places that open the lobby
    RefreshLobby();
}

void Lobby::UpdateGameList(QStandardItemModel* list) {
    game_list->clear();
    for (int i = 0; i < list->rowCount(); i++) {
        QStandardItem* parent = list->item(i, 0);
        for (int j = 0; j < parent->rowCount(); j++) {
            game_list->appendRow(parent->child(j)->clone());
        }
    }
    if (proxy) {
        proxy->UpdateGameList(game_list);
    }
}

QString Lobby::PasswordPrompt() {
    bool ok;
    const QString text =
        QInputDialog::getText(this, QStringLiteral("Password Required to Join"),
                              QStringLiteral("Password:"), QLineEdit::Password, QString(), &ok);
    return ok ? text : QString();
}

void Lobby::OnExpandRoom(const QModelIndex& index) {
    // TODO(vvanelslande)
    // QModelIndex member_index = proxy->index(index.row(), Column::MEMBER);
    // QVariantList member_list =
    //     proxy->data(member_index, LobbyItemMemberList::MemberListRole).toList();
}

void Lobby::OnJoinRoom(const QModelIndex& source) {
    if (const std::shared_ptr<Network::RoomMember> room_member = Network::GetRoomMember().lock()) {
        // Prevent the user from trying to join a room while they are already joining.
        if (room_member->GetState() == Network::RoomMember::State::Joining) {
            return;
        } else if (room_member->IsConnected() && !NetworkMessage::WarnDisconnect()) {
            // And ask if they want to leave the room if they are already in one.
            return;
        }
    }
    QModelIndex index = source;
    // If the user double clicks on a child row (aka the player list) then use the parent instead
    if (source.parent() != QModelIndex()) {
        index = source.parent();
    }
    if (!ui->nickname->hasAcceptableInput()) {
        NetworkMessage::ShowError(NetworkMessage::NICKNAME_NOT_VALID);
        return;
    }

    // Get a password to pass if the room is password protected
    QModelIndex password_index = proxy->index(index.row(), Column::ROOM_NAME);
    const bool has_password = proxy->data(password_index, LobbyItemName::PasswordRole).toBool();
    const std::string password = has_password ? PasswordPrompt().toStdString() : "";
    if (has_password && password.empty()) {
        return;
    }

    QModelIndex connection_index = proxy->index(index.row(), Column::HOST);
    const std::string nickname = ui->nickname->text().toStdString();
    const std::string ip =
        proxy->data(connection_index, LobbyItemHost::HostIPRole).toString().toStdString();
    const int port = proxy->data(connection_index, LobbyItemHost::HostPortRole).toInt();
    const std::string verify_UID =
        proxy->data(connection_index, LobbyItemHost::HostVerifyUIDRole).toString().toStdString();

    // Attempt to connect in a different thread
    QFuture<void> f = QtConcurrent::run([nickname, ip, port, password, verify_UID] {
        std::string token;
        if (!Settings::values.citra_username.empty() && !Settings::values.citra_token.empty()) {
            WebService::Client client(Settings::values.web_api_url, Settings::values.citra_username,
                                      Settings::values.citra_token);
            token = client.GetExternalJWT(verify_UID).returned_data;
            if (token.empty()) {
                LOG_ERROR(WebService, "Could not get external JWT, verification may fail");
            } else {
                LOG_INFO(WebService, "Successfully requested external JWT: size={}", token.size());
            }
        }
        if (std::shared_ptr<Network::RoomMember> room_member = Network::GetRoomMember().lock()) {
            room_member->Join(nickname, Service::CFG::GetConsoleIdHash(Core::System::GetInstance()),
                              ip.c_str(), port, 0, Network::NoPreferredMac, password, token);
        }
    });
    watcher->setFuture(f);

    // TODO(jroweboy): disable widgets and display a connecting while we wait

    // Save settings
    UISettings::values.nickname = ui->nickname->text();
    UISettings::values.ip = proxy->data(connection_index, LobbyItemHost::HostIPRole).toString();
    UISettings::values.port = proxy->data(connection_index, LobbyItemHost::HostPortRole).toString();
    Settings::Apply();
}

void Lobby::ResetModel() {
    model->clear();
    model->insertColumns(0, Column::TOTAL);
    model->setHeaderData(Column::EXPAND, Qt::Horizontal, QString(), Qt::DisplayRole);
    model->setHeaderData(Column::ROOM_NAME, Qt::Horizontal, QStringLiteral("Room Name"),
                         Qt::DisplayRole);
    model->setHeaderData(Column::GAME_NAME, Qt::Horizontal, QStringLiteral("Preferred Game"),
                         Qt::DisplayRole);
    model->setHeaderData(Column::HOST, Qt::Horizontal, QStringLiteral("Host"), Qt::DisplayRole);
    model->setHeaderData(Column::MEMBER, Qt::Horizontal, QStringLiteral("Players"),
                         Qt::DisplayRole);
}

void Lobby::RefreshLobby() {
    if (const std::shared_ptr<Core::AnnounceMultiplayerSession> session =
            announce_multiplayer_session.lock()) {
        ResetModel();
        ui->refresh_list->setEnabled(false);
        ui->refresh_list->setText(QStringLiteral("Refreshing"));
        room_list_watcher.setFuture(
            QtConcurrent::run([session] { return session->GetRoomList(); }));
    } else {
        // TODO(jroweboy): Display an error box about announce couldn't be started
    }
}

void Lobby::OnRefreshLobby() {
    const AnnounceMultiplayerRoom::RoomList new_room_list = room_list_watcher.result();
    for (const AnnounceMultiplayerRoom::Room& room : new_room_list) {
        // Find the icon for the game if this person owns that game.
        QPixmap smdh_icon;
        for (int r = 0; r < game_list->rowCount(); ++r) {
            const QModelIndex index = game_list->index(r, 0);
            const u64 game_id =
                game_list->data(index, GameListItemPath::ProgramIdRole).toULongLong();
            if (game_id != 0 && room.preferred_game_id == game_id) {
                smdh_icon = game_list->data(index, Qt::DecorationRole).value<QPixmap>();
            }
        }

        QList<QVariant> members;
        for (const AnnounceMultiplayerRoom::Room::Member& member : room.members) {
            members.append(QVariant::fromValue<LobbyMember>(LobbyMember{
                QString::fromStdString(member.username), QString::fromStdString(member.nickname),
                member.game_id, QString::fromStdString(member.game_name)}));
        }

        LobbyItem* first_item = new LobbyItem();
        QList<QStandardItem*> row({
            first_item,
            new LobbyItemName(room.has_password, QString::fromStdString(room.name)),
            new LobbyItemGame(room.preferred_game_id, QString::fromStdString(room.preferred_game),
                              smdh_icon),
            new LobbyItemHost(QString::fromStdString(room.owner), QString::fromStdString(room.ip),
                              room.port, QString::fromStdString(room.verify_UID)),
            new LobbyItemMemberList(members, room.max_player),
        });
        model->appendRow(row);
        // To make the rows expandable, add the member data as a child of the first column of the
        // rows with people in them and have qt set them to colspan after the model is finished
        // resetting
        if (!room.description.empty()) {
            first_item->appendRow(
                new LobbyItemDescription(QString::fromStdString(room.description)));
        }
        if (!room.members.empty()) {
            first_item->appendRow(new LobbyItemExpandedMemberList(members));
        }
    }

    // Re-enable the refresh button and resize the columns
    ui->refresh_list->setEnabled(true);
    ui->refresh_list->setText(QStringLiteral("Refresh List"));
    ui->room_list->header()->stretchLastSection();
    for (int i = 0; i < Column::TOTAL - 1; ++i) {
        ui->room_list->resizeColumnToContents(i);
    }

    // Set the member list child items to span all columns
    for (int i = 0; i < proxy->rowCount(); i++) {
        QStandardItem* parent = model->item(i, 0);
        for (int j = 0; j < parent->rowCount(); j++) {
            ui->room_list->setFirstColumnSpanned(j, proxy->index(i, 0), true);
        }
    }
}

LobbyFilterProxyModel::LobbyFilterProxyModel(QWidget* parent, QStandardItemModel* list)
    : QSortFilterProxyModel(parent), game_list(list) {}

void LobbyFilterProxyModel::UpdateGameList(QStandardItemModel* list) {
    game_list = list;
}

bool LobbyFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const {
    // Prioritize filters by fastest to compute

    // Pass over any child rows (aka row that shows the players in the room)
    if (sourceParent != QModelIndex()) {
        return true;
    }

    // Filter by filled rooms
    if (filter_full) {
        const QModelIndex members = sourceModel()->index(sourceRow, Column::MEMBER, sourceParent);
        const int player_count =
            sourceModel()->data(members, LobbyItemMemberList::MemberListRole).toList().size();
        const int max_players =
            sourceModel()->data(members, LobbyItemMemberList::MaxPlayerRole).toInt();
        if (player_count >= max_players) {
            return false;
        }
    }

    // Filter by search parameters
    if (!filter_search.isEmpty()) {
        const QModelIndex game_name =
            sourceModel()->index(sourceRow, Column::GAME_NAME, sourceParent);
        const QModelIndex room_name =
            sourceModel()->index(sourceRow, Column::ROOM_NAME, sourceParent);
        const QModelIndex host_name = sourceModel()->index(sourceRow, Column::HOST, sourceParent);
        const bool preferred_game_match = sourceModel()
                                              ->data(game_name, LobbyItemGame::GameNameRole)
                                              .toString()
                                              .contains(filter_search, filterCaseSensitivity());
        const bool room_name_match = sourceModel()
                                         ->data(room_name, LobbyItemName::NameRole)
                                         .toString()
                                         .contains(filter_search, filterCaseSensitivity());
        const bool username_match = sourceModel()
                                        ->data(host_name, LobbyItemHost::HostUsernameRole)
                                        .toString()
                                        .contains(filter_search, filterCaseSensitivity());
        if (!preferred_game_match && !room_name_match && !username_match) {
            return false;
        }
    }

    // Filter by game owned
    if (filter_owned) {
        const QModelIndex game_name =
            sourceModel()->index(sourceRow, Column::GAME_NAME, sourceParent);
        QList<QModelIndex> owned_games;
        for (int r = 0; r < game_list->rowCount(); ++r) {
            owned_games.append(QModelIndex(game_list->index(r, 0)));
        }
        const u64 current_id =
            sourceModel()->data(game_name, LobbyItemGame::TitleIDRole).toLongLong();
        if (current_id == 0) {
            // TODO: homebrew often doesn't have a title ID and this hides them
            return false;
        }
        bool owned = false;
        for (const QModelIndex& game : owned_games) {
            const u64 game_id = game_list->data(game, GameListItemPath::ProgramIdRole).toLongLong();
            if (current_id == game_id) {
                owned = true;
            }
        }
        if (!owned) {
            return false;
        }
    }

    return true;
}

void LobbyFilterProxyModel::sort(int column, Qt::SortOrder order) {
    sourceModel()->sort(column, order);
}

void LobbyFilterProxyModel::SetFilterOwned(bool filter) {
    filter_owned = filter;
    invalidate();
}

void LobbyFilterProxyModel::SetFilterFull(bool filter) {
    filter_full = filter;
    invalidate();
}

void LobbyFilterProxyModel::SetFilterSearch(const QString& filter) {
    filter_search = filter;
    invalidate();
}
