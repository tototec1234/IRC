#include "lifecycle/ConnectionHealthMonitor.hpp"

#include <ctime>
#include <sstream>
#include <utility>

#include "b/ReplyBuilder.hpp"

ConnectionHealthMonitor::HealthState::HealthState()
    : lastActivity(0),
      waitingForPong(false),
      lastPingSentAt(0),
      lastPongReceivedAt(0),
      expectedPongToken("") {}

ConnectionHealthMonitor::HealthState::HealthState(std::time_t activityTime)
    : lastActivity(activityTime),
      waitingForPong(false),
      lastPingSentAt(0),
      lastPongReceivedAt(0),
      expectedPongToken("") {}

ConnectionHealthMonitor::ConnectionHealthMonitor()
    : _timeoutSeconds(DEFAULT_TIMEOUT_SECONDS) {}

ConnectionHealthMonitor::ConnectionHealthMonitor(std::time_t timeoutSeconds)
    : _timeoutSeconds(timeoutSeconds) {}

ConnectionHealthMonitor::~ConnectionHealthMonitor() {}

void ConnectionHealthMonitor::updateActivity(int fd) {
  updateActivity(fd, std::time(NULL));
}

void ConnectionHealthMonitor::updateActivity(int fd, std::time_t now) {
  getOrCreateState(fd, now).lastActivity = now;
}

CommandResult ConnectionHealthMonitor::generatePing(int fd) {
  return generatePing(fd, std::time(NULL));
}

CommandResult ConnectionHealthMonitor::generatePing(int fd, std::time_t now) {
  CommandResult result;
  HealthState& state = getOrCreateState(fd, now);

  state.lastActivity = now;
  state.waitingForPong = true;
  state.lastPingSentAt = now;
  state.expectedPongToken = makePingToken(fd, now);
  result.addReply(fd, ReplyBuilder::ping(state.expectedPongToken));
  return result;
}

void ConnectionHealthMonitor::markPongReceived(int fd,
                                               const std::string& token) {
  markPongReceived(fd, token, std::time(NULL));
}

void ConnectionHealthMonitor::markPongReceived(int fd,
                                               const std::string& token,
                                               std::time_t now) {
  HealthState& state = getOrCreateState(fd, now);
  state.lastActivity = now;
  if (state.waitingForPong && token == state.expectedPongToken) {
    state.waitingForPong = false;
    state.lastPongReceivedAt = now;
    state.expectedPongToken.clear();
  }
}

bool ConnectionHealthMonitor::hasTimedOut(int fd) const {
  return hasTimedOut(fd, std::time(NULL));
}

bool ConnectionHealthMonitor::hasTimedOut(int fd, std::time_t now) const {
  HealthMap::const_iterator it = _clients.find(fd);
  if (it == _clients.end() || !it->second.waitingForPong) {
    return false;
  }
  return std::difftime(now, it->second.lastPingSentAt) > _timeoutSeconds;
}

std::vector<int> ConnectionHealthMonitor::collectTimedOutClients() const {
  return collectTimedOutClients(std::time(NULL));
}

std::vector<int> ConnectionHealthMonitor::collectTimedOutClients(
    std::time_t now) const {
  std::vector<int> timedOutClients;

  for (HealthMap::const_iterator it = _clients.begin(); it != _clients.end();
       ++it) {
    if (hasTimedOut(it->first, now)) {
      timedOutClients.push_back(it->first);
    }
  }
  return timedOutClients;
}

void ConnectionHealthMonitor::removeClient(int fd) { _clients.erase(fd); }

bool ConnectionHealthMonitor::isWaitingForPong(int fd) const {
  HealthMap::const_iterator it = _clients.find(fd);
  return it != _clients.end() && it->second.waitingForPong;
}

std::string ConnectionHealthMonitor::getExpectedPongToken(int fd) const {
  HealthMap::const_iterator it = _clients.find(fd);
  if (it == _clients.end()) {
    return "";
  }
  return it->second.expectedPongToken;
}

ConnectionHealthMonitor::HealthState&
ConnectionHealthMonitor::getOrCreateState(int fd, std::time_t now) {
  HealthMap::iterator it = _clients.find(fd);
  if (it == _clients.end()) {
    it = _clients.insert(std::make_pair(fd, HealthState(now))).first;
  }
  return it->second;
}

std::string ConnectionHealthMonitor::makePingToken(int fd,
                                                   std::time_t now) const {
  std::ostringstream oss;
  oss << "irc.local-" << fd << "-" << now;
  return oss.str();
}
