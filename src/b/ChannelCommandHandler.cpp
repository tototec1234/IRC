#include "b/ChannelCommandHandler.hpp"

#include "b/ReplyBuilder.hpp"
#include "c/Channel.hpp"
#include "c/Client.hpp"
#include "c/ServerState.hpp"
#include <climits>
#include <cstdlib>
#include <cerrno>

ChannelCommandHandler::ChannelCommandHandler() {}

ChannelCommandHandler::~ChannelCommandHandler() {}

CommandResult ChannelCommandHandler::handleKick(int fd, const Message& msg,
                                                ServerState& state,
                                                Client& client) {
  CommandResult result;
  if (!client.isRegistered()) {
    result.addReply(fd, ReplyBuilder::noRegistered(client));
    return result;
  }
  if (msg.getParamCount() < 2) {
    result.addReply(fd, ReplyBuilder::needMoreParams(client, "KICK"));
    return result;
  }

  const std::string& channelName = msg.getSingleParam(0);
  const std::string& targetNick = msg.getSingleParam(1);
  Channel* channel = state.getChannel(channelName);
  if (!channel) {
    result.addReply(fd, ReplyBuilder::noSuchChannel(client, channelName));
    return result;
  }
  if (!channel->hasMember(&client)) {
    result.addReply(fd, ReplyBuilder::notOnChannel(client, channelName));
    return result;
  }
  if (!channel->isOperator(&client)) {
    result.addReply(fd, ReplyBuilder::chanOpPrivsNeeded(client, channelName));
    return result;
  }

  Client* targetClient = state.getClientByNick(targetNick);
  if (!targetClient) {
    result.addReply(fd, ReplyBuilder::noSuchNick(client, targetNick));
    return result;
  }
  if (!channel->hasMember(targetClient)) {
    result.addReply(fd, ReplyBuilder::userNotInChannel(client, channelName));
    return result;
  }

  std::string reason = client.getNick();
  if (msg.hasParam(2)) {
    reason = msg.getSingleParam(2);
  }
  std::string kickMsg = ReplyBuilder::kick(client.getFullPrefix(), channelName,
                                           targetNick, reason);
  addRepliesToMembers(result, channel->getMembers(), kickMsg, -1);
  state.removeClientFromChannel(targetClient, channelName);
  return result;
}

CommandResult ChannelCommandHandler::handleInvite(int fd, const Message& msg,
                                                  ServerState& state,
                                                  Client& client) {
  CommandResult result;
  if (!client.isRegistered()) {
    result.addReply(fd, ReplyBuilder::noRegistered(client));
    return result;
  }
  if (msg.getParamCount() < 2) {
    result.addReply(fd, ReplyBuilder::needMoreParams(client, "INVITE"));
    return result;
  }

  const std::string& targetNick = msg.getSingleParam(0);
  const std::string& channelName = msg.getSingleParam(1);
  Client* targetClient = state.getClientByNick(targetNick);
  if (!targetClient) {
    result.addReply(fd, ReplyBuilder::noSuchNick(client, targetNick));
    return result;
  }

  Channel* channel = state.getChannel(channelName);
  if (!channel) {
    result.addReply(fd, ReplyBuilder::noSuchChannel(client, channelName));
    return result;
  }
  if (!channel->hasMember(&client)) {
    result.addReply(fd, ReplyBuilder::notOnChannel(client, channelName));
    return result;
  }
  if (channel->hasMember(targetClient)) {
    result.addReply(fd,
                    ReplyBuilder::userOnChannel(client, targetNick,
                                                channelName));
    return result;
  }
  if (channel->getModes().isInviteOnly() && !channel->isOperator(&client)) {
    result.addReply(fd, ReplyBuilder::chanOpPrivsNeeded(client, channelName));
    return result;
  }

  state.inviteClientToChannel(targetClient, channel);
  result.addReply(fd, ReplyBuilder::inviting(client, targetNick, channelName));
  result.addReply(targetClient->getFd(),
                  ReplyBuilder::invite(client.getFullPrefix(), targetNick,
                                       channelName));
  return result;
}

CommandResult ChannelCommandHandler::handleTopic(int fd, const Message& msg,
                                                 ServerState& state,
                                                 Client& client) {
  CommandResult result;
  if (!client.isRegistered()) {
    result.addReply(fd, ReplyBuilder::noRegistered(client));
    return result;
  }
  if (!msg.hasParam(0)) {
    result.addReply(fd, ReplyBuilder::needMoreParams(client, "TOPIC"));
    return result;
  }

  const std::string& channelName = msg.getSingleParam(0);
  Channel* channel = state.getChannel(channelName);
  if (!channel) {
    result.addReply(fd, ReplyBuilder::noSuchChannel(client, channelName));
    return result;
  }
  if (!channel->hasMember(&client)) {
    result.addReply(fd, ReplyBuilder::notOnChannel(client, channelName));
    return result;
  }

  if (!msg.hasParam(1)) {
    if (channel->getTopic().empty()) {
      result.addReply(fd, ReplyBuilder::noTopic(client, channelName));
    } else {
      result.addReply(fd, ReplyBuilder::topicReply(client, channelName,
                                                   channel->getTopic()));
    }
    return result;
  }

  if (channel->getModes().isTopicRestricted() && !channel->isOperator(&client)) {
    result.addReply(fd, ReplyBuilder::chanOpPrivsNeeded(client, channelName));
    return result;
  }

  const std::string& topic = msg.getSingleParam(1);
  channel->setTopic(topic);
  addRepliesToMembers(result, channel->getMembers(),
                      ReplyBuilder::topic(client.getFullPrefix(), "TOPIC",
                                          channelName, topic),
                      -1);
  return result;
}

CommandResult ChannelCommandHandler::handleMode(int fd, const Message& msg,
                                                ServerState& state,
                                                Client& client) {
  CommandResult result;
  if (!client.isRegistered()) {
    result.addReply(fd, ReplyBuilder::noRegistered(client));
    return result;
  }
  if (msg.getParamCount() < 2) {
    result.addReply(fd, ReplyBuilder::needMoreParams(client, "MODE"));
    return result;
  }

  const std::string& channelName = msg.getSingleParam(0);
  const std::string& modeToken = msg.getSingleParam(1);
  Channel* channel = state.getChannel(channelName);
  if (!channel) {
    result.addReply(fd, ReplyBuilder::noSuchChannel(client, channelName));
    return result;
  }
  if (!channel->hasMember(&client)) {
    result.addReply(fd, ReplyBuilder::notOnChannel(client, channelName));
    return result;
  }
  if (!channel->isOperator(&client)) {
    result.addReply(fd, ReplyBuilder::chanOpPrivsNeeded(client, channelName));
    return result;
  }

  // This implementation accepts one channel mode change per MODE command.
  if (!isSupportedSingleModeToken(modeToken)) {
    result.addReply(fd, ReplyBuilder::unknownMode(client, modeToken));
    return result;
  }

  const bool adding = modeToken[0] == '+';
  const char mode = modeToken[1];
  std::string modeArg;
  if (mode == 'i') {
    channel->getModes().setInviteOnly(adding);
  } else if (mode == 't') {
    channel->getModes().setTopicRestricted(adding);
  } else if (mode == 'k') {
    if (adding) {
      if (!msg.hasParam(2)) {
        result.addReply(fd, ReplyBuilder::needMoreParams(client, "MODE"));
        return result;
      }
      modeArg = msg.getSingleParam(2);
      channel->getModes().setKey(modeArg);
    } else {
      channel->getModes().unSetKey();
    }
  } else if (mode == 'l') {
    if (adding) {
      int limit = -1;
      if (!msg.hasParam(2)) {
        result.addReply(fd, ReplyBuilder::needMoreParams(client, "MODE"));
        return result;
      }
      modeArg = msg.getSingleParam(2);
      if (!parsePositiveLimit(modeArg, limit)) {
        result.addReply(fd, ReplyBuilder::unknownMode(client, modeToken));
        return result;
      }
      channel->getModes().setLimit(limit);
    } else {
      channel->getModes().unSetLimit();
    }
  } else if (mode == 'o') {
    return applyOperatorMode(fd, msg, state, client, *channel, channelName,
                             modeToken);
  }

  addRepliesToMembers(result, channel->getMembers(),
                      ReplyBuilder::mode(client.getFullPrefix(), channelName,
                                         modeToken, modeArg),
                      -1);
  return result;
}

CommandResult ChannelCommandHandler::applyOperatorMode(
    int fd, const Message& msg, ServerState& state, Client& client,
    Channel& channel, const std::string& channelName,
    const std::string& modeToken) {
  CommandResult result;
  if (!msg.hasParam(2)) {
    result.addReply(fd, ReplyBuilder::needMoreParams(client, "MODE"));
    return result;
  }

  const std::string& targetNick = msg.getSingleParam(2);
  Client* targetClient = state.getClientByNick(targetNick);
  if (!targetClient) {
    result.addReply(fd, ReplyBuilder::noSuchNick(client, targetNick));
    return result;
  }
  if (!channel.hasMember(targetClient)) {
    result.addReply(fd, ReplyBuilder::userNotInChannel(client, channelName));
    return result;
  }

  channel.setOperator(targetClient, modeToken[0] == '+');
  addRepliesToMembers(result, channel.getMembers(),
                      ReplyBuilder::mode(client.getFullPrefix(), channelName,
                                         modeToken, targetNick),
                      -1);
  return result;
}

void ChannelCommandHandler::addRepliesToMembers(
    CommandResult& result, const std::vector<Client*>& members,
    const std::string& message, int exceptFd) {
  for (std::vector<Client*>::const_iterator it = members.begin();
       it != members.end(); ++it) {
    Client* member = *it;
    if (member && member->getFd() != exceptFd) {
      result.addReply(member->getFd(), message);
    }
  }
}

bool ChannelCommandHandler::parsePositiveLimit(const std::string& value,
                                               int& limit) const {
  if (value.empty() || value[0] == '-') {
    return false;
  }

  char* end = NULL;
  errno = 0;
  long parsed = std::strtol(value.c_str(), &end, 10);

  if (*end != '\0' || errno == ERANGE || parsed > INT_MAX) {
    return false;
  }

  limit = static_cast<int>(parsed);
  return true;
}

bool ChannelCommandHandler::isSupportedSingleModeToken(
    const std::string& modeToken) const {
  if (modeToken.size() != 2) {
    return false;
  }
  if (modeToken[0] != '+' && modeToken[0] != '-') {
    return false;
  }
  const char mode = modeToken[1];
  return mode == 'i' || mode == 't' || mode == 'k' || mode == 'o' ||
         mode == 'l';
}
