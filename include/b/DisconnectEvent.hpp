#ifndef DISCONNECTEVENT_HPP
#define DISCONNECTEVENT_HPP

#include <string>

struct DisconnectEvent {
  int fd;
  std::string reason;

  DisconnectEvent();
  DisconnectEvent(int clientFd, const std::string& disconnectReason);
};

#endif
