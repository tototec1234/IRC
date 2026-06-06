#include "c/Channel.hpp"

#include "c/Client.hpp"

Channel::Channel(const std::string& name) : _name(name) {}

Channel::~Channel() {}

const std::string& Channel::getName() const { return _name; }

const std::string& Channel::getTopic() const { return _topic; }

void Channel::setTopic(const std::string& topic) { _topic = topic; }

bool Channel::hasMember(Client* client) const {
  return _members.find(client) != _members.end();
}

void Channel::_unsafe_addMember(Client* client) {
  _members[client] = MemberInfo();
  removeInvite(client);
}

void Channel::_unsafe_removeMember(Client* client) { _members.erase(client); }

void Channel::_unsafe_removeClientState(Client* client) {
  _unsafe_removeMember(client);
  removeInvite(client);
}

std::vector<Client*> Channel::getMembers() const {
  std::vector<Client*> memberList;
  memberList.reserve(_members.size());
  for (std::map<Client*, MemberInfo>::const_iterator it = _members.begin();
       it != _members.end(); ++it) {
    memberList.push_back(it->first);
  }
  return memberList;
}

size_t Channel::memberCount() const { return _members.size(); }

bool Channel::isOperator(Client* client) const {
  std::map<Client*, MemberInfo>::const_iterator it = _members.find(client);
  if (it != _members.end()) {
    return it->second.isOperator;
  }
  return false;
}

void Channel::setOperator(Client* client, bool isOperator) {
  std::map<Client*, MemberInfo>::iterator it = _members.find(client);
  if (it != _members.end()) {
    it->second.isOperator = isOperator;
  }
}

void Channel::addInvite(Client* client) { _invitedClients.insert(client); }
bool Channel::isInvited(Client* client) const {
  return _invitedClients.find(client) != _invitedClients.end();
}

void Channel::removeInvite(Client* client) { _invitedClients.erase(client); }
ChannelModes& Channel::getModes() { return _modes; }
const ChannelModes& Channel::getModes() const { return _modes; }
bool Channel::isEmpty() const { return _members.empty(); }
