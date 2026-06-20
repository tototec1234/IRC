#ifndef CONNECTIONHEALTHMONITOR_HPP
#define CONNECTIONHEALTHMONITOR_HPP

#include <ctime>
#include <map>
#include <string>
#include <vector>

#include "b/CommandResult.hpp"

class ConnectionHealthMonitor {
 public:
  static const std::time_t DEFAULT_TIMEOUT_SECONDS = 120;

  ConnectionHealthMonitor();
  explicit ConnectionHealthMonitor(std::time_t timeoutSeconds);
  ~ConnectionHealthMonitor();

  void updateActivity(int fd);
  void updateActivity(int fd, std::time_t now);

  CommandResult generatePing(int fd);
  CommandResult generatePing(int fd, std::time_t now);

  void markPongReceived(int fd, const std::string& token);
  void markPongReceived(int fd, const std::string& token, std::time_t now);

  bool hasTimedOut(int fd) const;
  bool hasTimedOut(int fd, std::time_t now) const;

  std::vector<int> collectTimedOutClients() const;
  std::vector<int> collectTimedOutClients(std::time_t now) const;

  bool isWaitingForPong(int fd) const;
  std::string getExpectedPongToken(int fd) const;

 private:
  struct HealthState {
    HealthState();
    explicit HealthState(std::time_t activityTime);

    std::time_t lastActivity;
    bool waitingForPong;
    std::time_t lastPingSentAt;
    std::time_t lastPongReceivedAt;
    std::string expectedPongToken;
  };

  typedef std::map<int, HealthState> HealthMap;

  std::time_t _timeoutSeconds;
  HealthMap _clients;

  HealthState& getOrCreateState(int fd, std::time_t now);
  std::string makePingToken(int fd, std::time_t now) const;

  ConnectionHealthMonitor(const ConnectionHealthMonitor&);
  ConnectionHealthMonitor& operator=(const ConnectionHealthMonitor&);
};

#endif
