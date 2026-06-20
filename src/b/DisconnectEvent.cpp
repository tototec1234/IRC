#include "b/DisconnectEvent.hpp"

DisconnectEvent::DisconnectEvent() : fd(-1), reason("") {}

DisconnectEvent::DisconnectEvent(int clientFd,
                                 const std::string& disconnectReason)
    : fd(clientFd), reason(disconnectReason) {}
