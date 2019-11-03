// Copyright 2017 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QMessageBox>
#include <QString>

#include "citra_qt/multiplayer/message.h"

namespace NetworkMessage {
const ConnectionError NICKNAME_NOT_VALID(
    "Nickname is not valid. Must be 4 to 20 alphanumeric characters.");
const ConnectionError ROOMNAME_NOT_VALID(
    "Room name is not valid. Must be 4 to 20 alphanumeric characters.");
const ConnectionError NICKNAME_NOT_VALID_SERVER(
    "Nickname is already in use or not valid. Please choose another.");
const ConnectionError IP_ADDRESS_NOT_VALID("IP is not a valid IPv4 address.");
const ConnectionError PORT_NOT_VALID("Port must be a number between 0 to 65535.");
const ConnectionError GAME_NOT_SELECTED(
    "You must choose a Preferred Game to host a room. If you do not have any games in your game "
    "list yet, add a game folder by clicking on the plus icon in the game list.");
const ConnectionError NO_INTERNET(
    "Unable to find an internet connection. Check your internet settings.");
const ConnectionError UNABLE_TO_CONNECT(
    "Unable to connect to the host. Verify that the connection settings are correct. If "
    "you still cannot connect, contact the room host and verify that the host is "
    "properly configured with the external port forwarded.");
const ConnectionError ROOM_IS_FULL("Unable to connect to the room because it is already full.");
const ConnectionError COULD_NOT_CREATE_ROOM(
    "Creating a room failed. Please retry. Restarting Citra might be necessary.");
const ConnectionError HOST_BANNED(
    "The host of the room has banned you. Speak with the host to unban you "
    "or try a different room.");
const ConnectionError WRONG_VERSION(
    "Version mismatch! Please update to the latest version of Citra Valentin. If the problem "
    "persists, contact the room host and ask them to update the server.");
const ConnectionError WRONG_PASSWORD("Incorrect password.");
const ConnectionError GENERIC_ERROR(
    "An unknown error occured. If this error continues to occur, please open an issue");
const ConnectionError LOST_CONNECTION("Connection to room lost. Try to reconnect.");
const ConnectionError HOST_KICKED("You have been kicked by the room host.");
const ConnectionError MAC_COLLISION("MAC address is already in use. Please choose another.");
const ConnectionError CONSOLE_ID_COLLISION(
    "Your Console ID conflicted with someone else's in the room.\n\nPlease go to Emulation "
    "> Configure > System to regenerate your Console ID.");
const ConnectionError PERMISSION_DENIED(
    "You do not have enough permission to perform this action.");
const ConnectionError NO_SUCH_USER(
    "The user you are trying to kick/ban could not be found.\nThey may have left the room.");

static bool WarnMessage(const std::string& title, const std::string& text) {
    return QMessageBox::Ok == QMessageBox::warning(nullptr, title.c_str(), text.c_str(),
                                                   QMessageBox::Ok | QMessageBox::Cancel);
}

void ShowError(const ConnectionError& e) {
    QMessageBox::critical(nullptr, QStringLiteral("Error"), e.GetString().c_str());
}

bool WarnCloseRoom() {
    return WarnMessage("Leave Room",
                       "You are about to close the room. Any network connections will be closed.");
}

bool WarnDisconnect() {
    return WarnMessage("Disconnect",
                       "You are about to leave the room. Any network connections will be closed.");
}

} // namespace NetworkMessage
