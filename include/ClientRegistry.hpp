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

  void addClient(int fd);
  void removeClient(int fd);
  Client* getClientByFd(int fd);
  Client* getClientByNick(const std::string& nick);
  // 単にnickExistsで存在を確認してからupdateNickを呼び出そうとすると
  // 自分自身と同じnickに変更しようとしたときalready existsと判断されてしまう
  // そのため、NICKコマンドについてupdatenickで対応することとする
  bool nickExists(const std::string& nick) const;
  bool updateNick(Client& client, const std::string& newNick);

 private:
  typedef std::map<std::string, Client*, IrcStringCompare> NicknameMap;
  typedef NicknameMap::iterator MapIt;
  NicknameMap _nickname_to_client;
  std::map<int, Client*> _fd_to_client;
};

#endif
