#ifndef CLIENTREGISTRY_HPP
#define CLIENTREGISTRY_HPP

#include <map>
#include <string>

#include "Client.hpp"

class ClientRegistry {
 public:
  ClientRegistry();
  ~ClientRegistry();

 private:
  std::map<std::string, Client*> _nickname_to_client;
  std::map<int, Client*> _fd_to_client;
};

#endif
