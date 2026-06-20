#ifndef DISCONNECTNOTIFIER_HPP
#define DISCONNECTNOTIFIER_HPP

#include "CommandResult.hpp"
#include "DisconnectEvent.hpp"

class ServerState;

class DisconnectNotifier {
 public:
  DisconnectNotifier();
  ~DisconnectNotifier();

  CommandResult build(const DisconnectEvent& event, ServerState& state);

 private:
  DisconnectNotifier(const DisconnectNotifier&);
  DisconnectNotifier& operator=(const DisconnectNotifier&);
};

#endif
