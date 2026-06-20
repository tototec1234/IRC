#ifndef CHANNELCOMMANDHANDLER_HPP
#define CHANNELCOMMANDHANDLER_HPP

#include "CommandResult.hpp"
#include "Message.hpp"
#include <string>
#include <vector>

class Channel;
class Client;
class ServerState;

class ChannelCommandHandler {
 public:
  ChannelCommandHandler();
  ~ChannelCommandHandler();

  CommandResult handleKick(int fd, const Message& msg, ServerState& state,
                           Client& client);
  CommandResult handleInvite(int fd, const Message& msg, ServerState& state,
                             Client& client);
  CommandResult handleTopic(int fd, const Message& msg, ServerState& state,
                            Client& client);
  CommandResult handleMode(int fd, const Message& msg, ServerState& state,
                           Client& client);

 private:
  void addRepliesToMembers(CommandResult& result,
                           const std::vector<Client*>& members,
                           const std::string& message, int exceptFd);
  bool parsePositiveLimit(const std::string& value, int& limit) const;
  bool isSupportedSingleModeToken(const std::string& modeToken) const;
  CommandResult applyOperatorMode(int fd, const Message& msg,
                                  ServerState& state, Client& client,
                                  Channel& channel,
                                  const std::string& channelName,
                                  const std::string& modeToken);

  ChannelCommandHandler(const ChannelCommandHandler&);
  ChannelCommandHandler& operator=(const ChannelCommandHandler&);
};

#endif
