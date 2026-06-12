#ifndef CLIENTREGISTRY_HPP
#define CLIENTREGISTRY_HPP

#include <map>
#include <string>

#include "Client.hpp"
#include "Utils.hpp"

class ClientRegistry {
 public:
  ClientRegistry();
  ~ClientRegistry();

  void addClient(int fd, const std::string& host);
  void removeClient(int fd);
  Client* getClientByFd(int fd);
  Client* getClientByNick(const std::string& nick);
  // ServerState::updateNick()の委譲先。
  // nick mapの重複検査、旧nick削除、新nick登録、Client cache更新を行う。
  bool nickExists(const std::string& nick) const;
  bool updateNick(Client& client, const std::string& newNick);

 private:
  typedef std::map<std::string, Client*, IrcStringCompare> NicknameMap;
  typedef NicknameMap::iterator MapIt;
  NicknameMap _nicknameToClient;
  std::map<int, Client*> _fdToClient;
};

#endif
