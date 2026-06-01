#ifndef CLIENTREGISTRY_HPP
#define CLIENTREGISTRY_HPP

#include <map>
#include <string>

#include "Client.hpp"

class ClientRegistry {
 public:
  ClientRegistry();
  ~ClientRegistry();

  void addClient(int fd);
  void removeClient(int fd);
  Client* getClientByFd(int fd);
  Client* getClientByNick(const std::string& nick);
  bool nickExists(const std::string& nick) const;
  bool updateNick(Client& client, const std::string& newNick);

 private:
  std::map<std::string, Client*> _nickname_to_client;
  std::map<int, Client*> _fd_to_client;
};

#endif
