#include "c/ClientRegistry.hpp"

#include <utility>

#include "c/Client.hpp"
#include "c/Utils.hpp"

ClientRegistry::ClientRegistry() {}
ClientRegistry::~ClientRegistry() {
  for (std::map<int, Client*>::iterator it = _fdToClient.begin();
       it != _fdToClient.end(); ++it) {
    delete it->second;
  }
}

void ClientRegistry::addClient(int fd, const std::string& host) {
  if (getClientByFd(fd)) {
    return;
  }
  Client* client = new Client(fd, host);
  _fdToClient.insert(std::make_pair(fd, client));
}

void ClientRegistry::removeClient(int fd) {
  Client* client = getClientByFd(fd);
  if (client) {
    _nicknameToClient.erase(client->getNick());
    _fdToClient.erase(fd);
    delete client;
  }
}

Client* ClientRegistry::getClientByFd(int fd) {
  std::map<int, Client*>::iterator it = _fdToClient.find(fd);
  if (it != _fdToClient.end()) {
    return it->second;
  }
  return NULL;
}

Client* ClientRegistry::getClientByNick(const std::string& nick) {
  std::map<std::string, Client*>::iterator it = _nicknameToClient.find(nick);
  if (it != _nicknameToClient.end()) {
    return it->second;
  }
  return NULL;
}

bool ClientRegistry::nickExists(const std::string& nick) const {
  return _nicknameToClient.find(nick) != _nicknameToClient.end();
}

bool ClientRegistry::updateNick(Client& client, const std::string& newNick) {
  std::string oldNick = client.getNick();

  // 1. 完全一致なら何もしない (早期リターン)
  if (oldNick == newNick) {
    return true;
  }

  // 2. 存在確認 (IrcStringCompareによる大文字小文字を無視した検索)
  MapIt it = _nicknameToClient.find(newNick);

  if (it != _nicknameToClient.end()) {
    // 存在するが、その所有者が自分自身ではない場合（＝他人が使っている）
    if (it->second != &client) {
      return false;  // 衝突 (ERR_NICKNAMEINUSE)
    }
    // 所有者が自分自身の場合は、単なる大文字小文字の変更（Alice ->
    // alice）なので続行
  }

  // 3. マップの更新
  if (!oldNick.empty()) {
    _nicknameToClient.erase(oldNick);
  }

  _nicknameToClient.insert(std::make_pair(newNick, &client));
  client._unsafe_setNick(newNick);

  return true;
}
