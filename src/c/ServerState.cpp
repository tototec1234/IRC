#include "ServerState.hpp"

#include <utility>

ServerState::ServerState(const std::string& password) : _password(password) {}

ServerState::~ServerState() {
  for (ChannelMapIt it = _channels.begin(); it != _channels.end(); ++it) {
    delete it->second;
  }
}

const std::string& ServerState::getPassword() const { return _password; }

void ServerState::addClient(int fd) { _client.addClient(fd); }

Channel* ServerState::addClientToChannel(Client* client,
                                         const std::string& channelName) {
  if (!client) {
    return NULL;
  }
  Channel* channel = getOrCreateChannel(channelName);
  if (!channel) {
    return NULL;
  }
  if (channel->hasMember(client)) {
    return channel;
  }
  channel->_unsafe_addMember(client);
  client->_unsafe_joinChannel(channel);
  if (channel->memberCount() == 1) {
    channel->setOperator(client, true);
  }
  return channel;
}

void ServerState::removeClientFromChannel(Client* client,
                                          const std::string& channelName) {
  if (!client) {
    return;
  }
  Channel* channel = getChannel(channelName);
  if (!channel || !channel->hasMember(client)) {
    return;
  }
  channel->_unsafe_removeClientState(client);
  client->_unsafe_leaveChannel(channel);
  removeChannelIfEmpty(channelName);
}

void ServerState::inviteClientToChannel(Client* client, Channel* channel) {
  if (!client || !channel) {
    return;
  }
  channel->addInvite(client);
}

void ServerState::removeInviteFromChannel(Client* client, Channel* channel) {
  if (!client || !channel) {
    return;
  }
  channel->removeInvite(client);
}

void ServerState::removeClientFromAllInvites(Client* client) {
  if (!client) {
    return;
  }
  for (ChannelMapIt it = _channels.begin(); it != _channels.end(); ++it) {
    it->second->removeInvite(client);
  }
}

void ServerState::removeClient(int fd) {
  Client* client = _client.getClientByFd(fd);
  if (!client) {
    return;  // client not found
  }

  std::vector<Channel*> channels = client->getChannels();
  std::vector<Channel*>::iterator it = channels.begin();
  while (it != channels.end()) {
    Channel* channel = *it;
    std::string channelName = channel->getName();
    channel->_unsafe_removeClientState(client);
    client->_unsafe_leaveChannel(channel);
    removeChannelIfEmpty(channelName);
    ++it;
  }

  removeClientFromAllInvites(client);
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
  ChannelMapIt it = _channels.find(name);
  if (it != _channels.end()) {
    return it->second;
  }
  return NULL;
}

Channel* ServerState::getOrCreateChannel(const std::string& name) {
  Channel* channel = getChannel(name);
  if (!channel) {
    channel = new Channel(name);
    _channels.insert(std::make_pair(name, channel));
  }
  return channel;
}

void ServerState::removeChannelIfEmpty(const std::string& name) {
  ChannelMapIt it = _channels.find(name);
  if (it != _channels.end() && it->second->isEmpty()) {
    delete it->second;
    _channels.erase(it);
  }
}
