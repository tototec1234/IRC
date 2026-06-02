#include "ClientRegistry.hpp"

#include <utility>

#include "Client.hpp"
#include "Utils.hpp"

ClientRegistry::ClientRegistry() {}
ClientRegistry::~ClientRegistry() {
  for (std::map<int, Client*>::iterator it = _fd_to_client.begin();
       it != _fd_to_client.end(); ++it) {
    delete it->second;
  }
}

void ClientRegistry::addClient(int fd) {
  if (getClientByFd(fd)) {
    return;
  }
  Client* client = new Client(fd);
  _fd_to_client.insert(std::make_pair(fd, client));
}

void ClientRegistry::removeClient(int fd) {
  Client* client = getClientByFd(fd);
  if (client) {
    _nickname_to_client.erase(client->getNick());
    _fd_to_client.erase(fd);
    delete client;
  }
}

Client* ClientRegistry::getClientByFd(int fd) {
  std::map<int, Client*>::iterator it = _fd_to_client.find(fd);
  if (it != _fd_to_client.end()) {
    return it->second;
  }
  return NULL;
}

Client* ClientRegistry::getClientByNick(const std::string& nick) {
  std::map<std::string, Client*>::iterator it = _nickname_to_client.find(nick);
  if (it != _nickname_to_client.end()) {
    return it->second;
  }
  return NULL;
}

bool ClientRegistry::nickExists(const std::string& nick) const {
  return _nickname_to_client.find(nick) != _nickname_to_client.end();
}

/*
  testscenario
  1. oldnick is newnick
  2. newnick already exists (another client has the same nick)
  3. oldnick is empty (first time setting nick)
  4. normal case
*/
bool ClientRegistry::updateNick(Client& client, const std::string& newNick) {
  std::string oldNick = client.getNick();

  // 1. 完全一致なら何もしない (早期リターン)
  if (oldNick == newNick) {
    return true;
  }

  // 2. 存在確認 (IrcStringCompareによる大文字小文字を無視した検索)
  MapIt it = _nickname_to_client.find(newNick);

  if (it != _nickname_to_client.end()) {
    // 存在するが、その所有者が自分自身ではない場合（＝他人が使っている）
    if (it->second != &client) {
      return false;  // 衝突 (ERR_NICKNAMEINUSE)
    }
    // 所有者が自分自身の場合は、単なる大文字小文字の変更（Alice ->
    // alice）なので続行
  }

  // 3. マップの更新
  if (!oldNick.empty()) {
    _nickname_to_client.erase(oldNick);
  }

  _nickname_to_client.insert(std::make_pair(newNick, &client));
  client._unsafe_setNick(newNick);

  return true;
}
