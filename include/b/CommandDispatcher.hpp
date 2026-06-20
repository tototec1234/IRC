#ifndef COMMANDDISPATCHER_HPP
#define COMMANDDISPATCHER_HPP

#include "CommandResult.hpp"
#include "Message.hpp"
#include "b/ChannelCommandHandler.hpp"
#include <string>
#include <vector>

class ServerState;
class Client;
class ConnectionHealthMonitor;

class CommandDispatcher {
 public:
  CommandDispatcher();
  ~CommandDispatcher();

  /**
   * @brief Executes one parsed IRC command for fd.
   *
   * Dispatcher owns IRC command semantics: it validates Message params, uses
   * ServerState/Client/Channel public APIs to update state, and returns
   * CommandResult for A layer to apply. It must not call send(), close(), or C
   * layer internal _unsafe_* APIs.
   */
  CommandResult dispatch(int fd, const Message& msg, ServerState& state);
  CommandResult dispatch(int fd, const Message& msg, ServerState& state,
                         ConnectionHealthMonitor& healthMonitor);

 private:
  /*
   * REVIEW: Before adding JOIN/PRIVMSG/MODE, consider replacing the current
   * if/else dispatch with a command table of member-function pointers.
  */
  CommandResult handlePass(int fd, const Message& msg, ServerState& state,
                           Client& client);
  CommandResult dispatch(int fd, const Message& msg, ServerState& state,
                         ConnectionHealthMonitor* healthMonitor);
  CommandResult handleNick(int fd, const Message& msg, ServerState& state,
                           Client& client);
  CommandResult handleUser(int fd, const Message& msg, Client& client);
  CommandResult handleJoin(int fd, const Message& msg, ServerState& state,
                           Client& client);
  CommandResult handlePart(int fd, const Message& msg,ServerState& state,
                           Client& client);
  CommandResult handlePrivmsg(int fd, const Message& msg,ServerState& state,
                              Client& client);
  CommandResult handleNotice(int fd, const Message& msg,ServerState& state,
                             Client& client);
  CommandResult handleTextMessage(int fd, const Message& msg,
                                  ServerState& state, Client& client,
                                  const std::string& command,
                                  bool replyOnError);
  CommandResult handleQuit(int fd, const Message& msg, ServerState& state,
                           Client& client);
  CommandResult handlePong(int fd, const Message& msg, Client& client,
                           ConnectionHealthMonitor* healthMonitor);
  /*
   * Registration completion is intentionally separate from PASS. Current B
   * policy requires PASS before NICK/USER can mutate Client registration data.
   */
  void maybeRegister(Client& client, CommandResult& result);
  void addRepliesToMembers(CommandResult& result,
                           const std::vector<Client*>& members,
                           const std::string& message, int exceptFd);

  ChannelCommandHandler _channelCommandHandler;

  // Copying is disabled explicitly.
  CommandDispatcher(const CommandDispatcher&);
  CommandDispatcher& operator=(const CommandDispatcher&);
};

#endif
