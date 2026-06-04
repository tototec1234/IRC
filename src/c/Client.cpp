#include "Client.hpp"

#include <vector>

Client::Client(int fd)
    : _nickname(""),
      _username(""),
      _realname(""),
      _host(""),
      _fd(fd),
      _passOk(false),
      _registered(false) {}

Client::~Client() {}

int Client::getFd() const { return _fd; }
const std::string& Client::getNick() const { return _nickname; }
const std::string& Client::getUsername() const { return _username; }
const std::string& Client::getRealname() const { return _realname; }
const std::string& Client::getHost() const { return _host; }
std::vector<Channel*> Client::getChannels() const {
  return std::vector<Channel*>(_channels.begin(), _channels.end());
}
std::string Client::getFullPrefix() const {
  return _nickname + "!" + _username + "@" + _host;
}

void Client::setUsername(const std::string& username) { _username = username; }
void Client::setRealname(const std::string& realname) { _realname = realname; }
void Client::setHost(const std::string& host) { _host = host; }
void Client::_unsafe_setNick(const std::string& nick) { _nickname = nick; }
void Client::_unsafe_joinChannel(Channel* channel) { _channels.insert(channel); }
void Client::_unsafe_leaveChannel(Channel* channel) { _channels.erase(channel); }
void Client::setPassOk(bool passOk) { _passOk = passOk; }
bool Client::isPassOk() const { return _passOk; }
bool Client::isRegistered() const { return _registered; }
bool Client::canRegister() const {
  return _passOk && !_nickname.empty() && !_username.empty() &&
         !_realname.empty();
}
void Client::markRegistered() { _registered = true; }
