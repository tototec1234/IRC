#include "ServerState.hpp"

ServerState::ServerState(const std::string& password) : _password(password) {}

ServerState::~ServerState() {
  // clean up channels and clients?
}

const std::string& ServerState::getPassword() const { return _password; }

void ServerState::addclient(int fd) { _client.addClient(fd); }

void ServerState::removeclient(int fd) {
  Client* client = _client.getClientByFd(fd);
  if (!client) {
    return;  // client not found
  }

  // Remove client from all channels
  // Have to rewrite following C++98 style
  for (Channel* channel : client->getChannels()) {
    channel->removeClient(client);
    if (channel->isEmpty()) {
      removeChannel(channel->getName());
    }
  }

  _client.removeClient(fd);
}

Client* ServerState::getClientByFd(int fd) { return _client.getClientByFd(fd); }

Client* ServerState::getClientByNick(const std::string& nick) {
  return _client.getClientByNick(nick);
}

bool ServerState::nickExists(const std::string& nick) const {
  return _client.nickExists(nick);
}

bool ServerState::updateNick(Client& client, const std::string& newNick) {
  return _client.updateNick(client, newNick);
}

Channel* ServerState::getChannel(const std::string& name) {
  auto it = _channels.find(name);
  if (it != _channels.end()) {
    return it->second;
  }
  return nullptr;
}

Channel* ServerState::getOrCreateChannel(const std::string& name) {
  Channel* channel = getChannel(name);
  if (!channel) {
    channel = new Channel(name);
    _channels[name] = channel;
  }
  return channel;
}

void ServerState::removeChannel(const std::string& name) {
  auto it = _channels.find(name);
  if (it != _channels.end()) {
    delete it->second;
    _channels.erase(it);
  }
}
