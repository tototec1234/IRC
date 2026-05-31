#ifndef SERVERSTATE_HPP
#define SERVERSTATE_HPP

#include <map>
#include <string>

#include "Channel.hpp"
#include "Client.hpp"
#include "ClientRegistry.hpp"

class ServerState {
 public:
  ServerState();
  ~ServerState();

  const std::string& getPassword() const;

 private:
  std::string _password;
  std::map<std::string, Channel*> _channels;
  ClientRegistry _client;
};

#endif
