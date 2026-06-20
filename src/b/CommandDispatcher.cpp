#include "b/CommandDispatcher.hpp"
#include "b/ReplyBuilder.hpp"
#include "c/ServerState.hpp"
#include "lifecycle/ConnectionHealthMonitor.hpp"
#include <iterator>
// #include <type_traits>　c++11 なぜまぎれこんでる？
#include <vector>

// #include "CommandDispatcher.hpp"

// #include "Client.hpp"
// #include "ReplyBuilder.hpp"
// #include "ServerState.hpp"

CommandDispatcher::CommandDispatcher() {}

CommandDispatcher::~CommandDispatcher() {}

CommandResult CommandDispatcher::dispatch(int fd, const Message& msg,
                                          ServerState& state) {
  return dispatch(fd, msg, state, NULL);
}

CommandResult CommandDispatcher::dispatch(int fd, const Message& msg,
                                          ServerState& state,
                                          ConnectionHealthMonitor& healthMonitor) {
  return dispatch(fd, msg, state, &healthMonitor);
}

CommandResult CommandDispatcher::dispatch(int fd, const Message& msg,
                                          ServerState& state,
                                          ConnectionHealthMonitor* healthMonitor) {
  Client* client = state.getClientByFd(fd);
  CommandResult result;
  const std::string& command = msg.getCommand();

  if (command == "PING") {
    if (!msg.hasParam(0)) {
      result.addReply(fd, ReplyBuilder::needMoreParams(client, command));
    } else {
      result.addReply(fd, ReplyBuilder::pong(msg.getSingleParam(0)));
    }
  } else if (command == "QUIT") {
	result = handleQuit(fd, msg, state, client);
  } else if (command == "PASS") {
    result = handlePass(fd, msg, state, client);
  } else if (command == "NICK") {
    if (!client->isPassOk()) {
      result.addReply(fd, ReplyBuilder::passwordMismatch());
      return result;
    }
    result = handleNick(fd, msg, state, client);
  } else if (command == "USER") {
    if (!client->isPassOk()) {
      result.addReply(fd, ReplyBuilder::passwordMismatch());
      return result;
    }
    result = handleUser(fd, msg, client);
  } else if (command == "JOIN") {
	result = handleJoin(fd, msg, state, client);
  } else if (command == "PART") {
	result = handlePart(fd, msg, state, client);
  } else if (command == "PRIVMSG") {
	result = handlePrivmsg(fd, msg, state, client);
  } else if (command == "NOTICE") {
	result = handleNotice(fd, msg, state, client);
//   } else if (command == "KICK") {
// 	result = handleKick(fd, msg, state, client);
  } else if (command == "INVITE") {
	result = handleInvite(fd, msg, state, client);
  } else if (command == "TOPIC") {
	result = handleTopic(fd, msg, state, client);
//   } else if (command == "MODE") {
// 	result = handleMode(fd, msg, state, client);
  } else if (command == "PONG") {
	result = handlePong(fd, msg, client, healthMonitor);
  } else if (!command.empty()) {
    result.addReply(fd, ReplyBuilder::unknownCommand(client, command));
  }
  return result;
}

CommandResult CommandDispatcher::handlePass(int fd, const Message& msg,
                                            ServerState& state,
                                            Client* client) {
  CommandResult result;
  if (!client) {
    return result;
  }
  if (!msg.hasParam(0)) {
    result.addReply(fd, ReplyBuilder::needMoreParams(client, "PASS"));
    return result;
  }
  if (client->isRegistered()) {
    result.addReply(fd, ReplyBuilder::alreadyRegistered(*client));
    return result;
  }
  if (msg.getSingleParam(0) != state.getPassword()) {
    result.addReply(fd, ReplyBuilder::passwordMismatch());
    return result;
  }
  client->setPassOk(true);
  return result;
}

CommandResult CommandDispatcher::handleNick(int fd, const Message& msg,
                                            ServerState& state,
                                            Client* client) {
  CommandResult result;
  if (!client) {
    return result;
  }
  if (!msg.hasParam(0)) {
    result.addReply(fd, ReplyBuilder::needMoreParams(client, "NICK"));
    return result;
  }
//   if (!client->isPassOk()) {
//     result.addReply(fd, ReplyBuilder::passwordMismatch());
//     return result;
//   }
  const std::string& nick = msg.getSingleParam(0);
  if (!state.updateNick(*client, nick)) {
    result.addReply(fd, ReplyBuilder::nickInUse(nick));
    return result;
  }
  maybeRegister(*client, result);
  return result;
}

CommandResult CommandDispatcher::handleUser(int fd, const Message& msg,
                                            Client* client) {
  CommandResult result;
  if (!client) {
    return result;
  }
  if (msg.getParamCount() < 4) {
    result.addReply(fd, ReplyBuilder::needMoreParams(client, "USER"));
    return result;
  }
//   if (!client->isPassOk()) {
//     result.addReply(fd, ReplyBuilder::passwordMismatch());
//     return result;
//   }
  if (client->isRegistered()) {
    result.addReply(fd, ReplyBuilder::alreadyRegistered(*client));
    return result;
  }
  client->setUsername(msg.getSingleParam(0));
  client->setRealname(msg.getSingleParam(3));
  maybeRegister(*client, result);
  return result;
}

// "JOIN"
CommandResult CommandDispatcher::handleJoin(int fd, const Message& msg,
                                            ServerState& state,
                                            Client* client) {
  CommandResult result;
  if (!client) {
    return result;
  }
//   if (!client->isPassOk()) {
//     result.addReply(fd, ReplyBuilder::passwordMismatch());
//     return result;
//   }
  if (!client->isRegistered()) {
    // result.addReply(fd, ReplyBuilder::alreadyRegistered(*client));
    result.addReply(fd, ReplyBuilder::noRegistered(*client));
    return result;
  }
  if (!msg.hasParam(0)) {
    result.addReply(fd, ReplyBuilder::needMoreParams(client, "JOIN"));
    return result;
  }
  const std::string channelName = msg.getSingleParam(0);

  Channel *channel = state.getChannel(channelName);
  if (channel && !channel->hasMember(client) &&
      channel->getModes().isInviteOnly() &&
      !channel->isInvited(client)) {
    result.addReply(fd, ReplyBuilder::inviteOnlyChan(*client, channelName));
    return result;
  }

  channel = state.addClientToChannel(client, channelName);
  if (!channel) {
	result.addReply(fd, ReplyBuilder::torima_Missing(*client, "JOIN"));
  	return result;
  }

  std::string joinMsg = ReplyBuilder::join(client->getFullPrefix(), "JOIN", channelName);
  addRepliesToMembers(result, channel->getMembers(), joinMsg, -1);

  return result;
}

// "PART"
CommandResult CommandDispatcher::handlePart(int fd, const Message& msg,
                                            ServerState& state,
                                            Client* client) {
  CommandResult result;
  if (!client) {
    return result;
  }
//   if (!client->isPassOk()) {
//     result.addReply(fd, ReplyBuilder::passwordMismatch());
//     return result;
//   }
  if (!client->isRegistered()) {
    result.addReply(fd, ReplyBuilder::noRegistered(*client));
    return result;
  }
  if (!msg.hasParam(0)) {
    result.addReply(fd, ReplyBuilder::needMoreParams(client, "PART"));
    return result;
  }

  const std::string channelName = msg.getSingleParam(0);
  Channel* channel = state.getChannel(channelName);
  if (!channel) {
    result.addReply(fd, ReplyBuilder::noSuchChannel(*client, channelName));
    return result;
  }
  if (!channel->hasMember(client)) {
    result.addReply(fd, ReplyBuilder::notOnChannel(*client, channelName));
    return result;
  }
  std::string partMsg = ReplyBuilder::part(client->getFullPrefix(), "PART", channelName);
  std::vector<Client*> members = channel->getMembers();
  addRepliesToMembers(result, members, partMsg, -1);
  state.removeClientFromChannel(client, channelName);
  return result;
}

// "PRIVMSG"
CommandResult CommandDispatcher::handlePrivmsg(int fd, const Message& msg,
                                               ServerState& state,
                                               Client* client) {
  return handleTextMessage(fd, msg, state, client, "PRIVMSG", true);
}

// "NOTICE"
CommandResult CommandDispatcher::handleNotice(int fd, const Message& msg,
                                              ServerState& state,
                                              Client* client) {
  return handleTextMessage(fd, msg, state, client, "NOTICE", false);
}

CommandResult CommandDispatcher::handleTextMessage(int fd, const Message& msg,
                                                   ServerState& state,
                                                   Client* client,
                                                   const std::string& command,
                                                   bool replyOnError) {
  CommandResult result;
  if (!client) {
    return result;
  }
  if (!client->isRegistered()) {
    if (replyOnError) {
      result.addReply(fd, ReplyBuilder::noRegistered(*client));
    }
    return result;
  }
  if (msg.getParamCount() < 2) {
    if (replyOnError) {
      result.addReply(fd, ReplyBuilder::needMoreParams(client, command));
    }
    return result;
  }

  const std::string& targetName = msg.getSingleParam(0);
  const std::string& text   = msg.getSingleParam(1);

  std::string message;
  if (command == "NOTICE") {
    message =
        ReplyBuilder::notice(client->getFullPrefix(), command, targetName, text);
  } else {
    message =
        ReplyBuilder::privmsg(client->getFullPrefix(), command, targetName, text);
  }

  if (targetName.empty() || targetName[0] == '#') {
      Channel* channel = state.getChannel(targetName);
      if (!channel) {
      if (replyOnError) {
        result.addReply(fd, ReplyBuilder::noSuchChannel(*client, targetName));
      }
      return result;
      }
      if (!channel->hasMember(client)) {
      if (replyOnError) {
        result.addReply(fd, ReplyBuilder::cannotSendToChan(*client, targetName));
      }
      return result;
      }
    addRepliesToMembers(result, channel->getMembers(), message, fd);
  } else {
      Client* targetClient = state.getClientByNick(targetName);
    if (!targetClient) {
      if (replyOnError) {
        result.addReply(fd, ReplyBuilder::noSuchNick(*client, targetName));
      }
        return result;
    }
      result.addReply(targetClient->getFd(), message);
  }
  return result;
}

// "QUIT"
CommandResult CommandDispatcher::handleQuit(int, const Message&,
                                            ServerState&,
                                            Client* client) {
  CommandResult result;
  if (!client) {
    return result;
  }
  result.shouldDisconnect = true;
  return result;
}

CommandResult CommandDispatcher::handlePong(
    int fd, const Message& msg, Client* client,
    ConnectionHealthMonitor* healthMonitor) {
  CommandResult result;
  if (!client) {
    return result;
  }
  if (!msg.hasParam(0)) {
    result.addReply(fd, ReplyBuilder::needMoreParams(client, "PONG"));
    return result;
  }
  if (healthMonitor) {
    healthMonitor->markPongReceived(
        fd, msg.getSingleParam(msg.getParamCount() - 1));
  }
  return result;
}

// CommandResult CommandDispatcher::handleQuit(Client* client) {
//   CommandResult result;
//   if (!client) {
//     return result;
//   }
//   result.shouldDisconnect = true;
//   return result;
// }


// "KICK"
// CommandResult CommandDispatcher::handleKick(int fd, const Message& msg,
//                                             ServerState& state,
//                                             Client* client) {
//   CommandResult result;
//   if (!client) {
//     return result;
//   }
//   if (!client->isRegistered()) {
//     result.addReply(fd, ReplyBuilder::noRegistered(*client));
//     return result;
//   }
//   if (msg.getParamCount() < 2) {
//     result.addReply(fd, ReplyBuilder::needMoreParams(client, "INVITE"));
//     return result;
//   }

//   const std::string& channelName = msg.getSingleParam(0);
//   const std::string& targetNick = msg.getSingleParam(1);

//   Channel* channel = state.getChannel(channelName);
//   if (!channel) {
//     result.addReply(fd, ReplyBuilder::noSuchChannel(*client, channelName));
//     return result;
//   }
//   if (!channel->hasMember(client)) {
//     result.addReply(fd, ReplyBuilder::notOnChannel(*client, channelName));
//     return result;
//   }

//   if (!channel->isOperator(client)) {
//     result.addReply(fd, ReplyBuilder::chanOpPrivsNeeded(*client, channelName));
//     return result;
//   }

//   return result;
// }

// "INVITE"
CommandResult CommandDispatcher::handleInvite(int fd, const Message& msg,
                                              ServerState& state,
                                              Client* client) {
  CommandResult result;
  if (!client) {
    return result;
  }
  if (!client->isRegistered()) {
    result.addReply(fd, ReplyBuilder::noRegistered(*client));
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
    result.addReply(fd, ReplyBuilder::noSuchNick(*client, targetNick));
    return result;
  }

  Channel* channel = state.getChannel(channelName);
  if (!channel) {
    result.addReply(fd, ReplyBuilder::noSuchChannel(*client, channelName));
    return result;
  }
  if (!channel->hasMember(client)) {
    result.addReply(fd, ReplyBuilder::notOnChannel(*client, channelName));
    return result;
  }
  if (channel->hasMember(targetClient)) {
    result.addReply(fd, ReplyBuilder::userOnChannel(*client, targetNick,
                                                    channelName));
    return result;
  }
  //	MODEが "+i" invite-onlyだったとき、オペレーターじゃなければ "482"
  if (channel->getModes().isInviteOnly() && !channel->isOperator(client)) {
    result.addReply(fd, ReplyBuilder::chanOpPrivsNeeded(*client, channelName));
    return result;
  }

  state.inviteClientToChannel(targetClient, channel);
  result.addReply(fd, ReplyBuilder::inviting(*client, targetNick, channelName));
  result.addReply(targetClient->getFd(), ReplyBuilder::invite(client->getFullPrefix(), targetNick, channelName));
  return result;
}

// "TOPIC"
CommandResult CommandDispatcher::handleTopic(int fd, const Message& msg,
                                             ServerState& state,
                                             Client* client) {
  CommandResult result;
  if (!client) {
      return result;
  }
  if (!client->isRegistered()) {
    result.addReply(fd, ReplyBuilder::noRegistered(*client));
    return result;
  }
  if (!msg.hasParam(0)) {
    result.addReply(fd, ReplyBuilder::needMoreParams(client, "TOPIC"));
    return result;
  }

  const std::string& channelName = msg.getSingleParam(0);
  Channel* channel = state.getChannel(channelName);
  if (!channel) {
    result.addReply(fd, ReplyBuilder::noSuchChannel(*client, channelName));
    return result;
  }
  if (!channel->hasMember(client)) {
    result.addReply(fd, ReplyBuilder::notOnChannel(*client, channelName));
    return result;
  }

  //	今のtopicを出力するパート！！
  if (!msg.hasParam(1)) {
    if (channel->getTopic().empty()) {
      result.addReply(fd, ReplyBuilder::noTopic(*client, channelName));
    } else {
      result.addReply(fd, ReplyBuilder::topicReply(*client, channelName,
                                                   channel->getTopic()));
    }
    return result;
  }

  //	MODEコマンド用の処理！！ "+t" かな？
  //	 "topic変更をチャンネルオペレーターだけに制限する" みたいな！
  if (channel->getModes().isTopicRestricted() && !channel->isOperator(client)) {
    result.addReply(fd, ReplyBuilder::chanOpPrivsNeeded(*client, channelName));
    return result;
  }

  //	topic書き換えのパート！！
  const std::string& topic = msg.getSingleParam(1);
  channel->setTopic(topic);

  //	ブロードキャスト！
  std::string topicMsg = ReplyBuilder::topic(client->getFullPrefix(), "TOPIC",channelName, topic);
  addRepliesToMembers(result, channel->getMembers(), topicMsg, -1);

  return result;
}

// "MODE"
// CommandResult CommandDispatcher::handleMode(int fd, const Message& msg,
//                                             ServerState& state,
//                                             Client* client) {
// }

// "PONG"
// CommandResult CommandDispatcher::handlePong(int fd, const Message& msg,
//                                             ServerState& state,
//                                             Client* client) {
// }






void CommandDispatcher::maybeRegister(Client& client, CommandResult& result) {
  if (!client.isRegistered() && client.canRegister()) {
    client.markRegistered();
    result.addReply(client.getFd(), ReplyBuilder::welcome(client));
  }
}

void CommandDispatcher::addRepliesToMembers(
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
