#include "b/CommandDispatcher.hpp"
#include "b/ReplyBuilder.hpp"
#include "c/ServerState.hpp"
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
  Client* client = state.getClientByFd(fd);
  CommandResult result;
  const std::string& command = msg.getCommand();

  if (command == "PING") {
    if (!msg.hasParam(0)) {
      result.addReply(fd, ReplyBuilder::needMoreParams(client, command));
    } else {
      result.addReply(fd, ReplyBuilder::pong(msg.getSingleParam(0)));
    }
  } else if (command == "PASS") {
    result = handlePass(fd, msg, state, client);
  } else if (!client->isPassOk()) {
    result.addReply(fd, ReplyBuilder::passwordMismatch());
    return result;
  } else if (command == "NICK") {
    result = handleNick(fd, msg, state, client);
  } else if (command == "USER") {
    result = handleUser(fd, msg, client);
  } else if (command == "JOIN") {
	result = handleJoin(fd, msg, state, client);
  } else if (command == "PART") {
	result = handlePart(fd, msg, state, client);
  } else if (command == "PRIVMSG") {
	result = handlePrivmsg(fd, msg, state, client);
//   } else if (command == "NOTICE") {
// 	result = handleNotice(fd, msg, state, client);
//   } else if (command == "QUIT") {
// 	result = handleQuit(fd, msg, state, client);
//   } else if (command == "KICK") {
// 	result = handleKick(fd, msg, state, client);
//   } else if (command == "INVITE") {
// 	result = handleInvite(fd, msg, state, client);
//   } else if (command == "TOPIC") {
// 	result = handleTopic(fd, msg, state, client);
//   } else if (command == "MODE") {
// 	result = handleMode(fd, msg, state, client);
//   } else if (command == "PONG") {
// 	result = handlePong(fd, msg, state, client);
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

//
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


  Channel *channel = state.addClientToChannel(client, channelName);
  if (!channel) {
	result.addReply(fd, ReplyBuilder::torima_Missing(*client, "JOIN"));
  	return result;
  }

  std::string joinMsg = ReplyBuilder::join(channelName, client->getFullPrefix(), "JOIN");

  std::vector<Client*> members = channel->getMembers();	// ディープコピーじゃなくていいのかな？

  for (std::vector<Client*>::iterator it = members.begin(); it != members.end(); ++it) {
      Client * client = *it;
	  result.addReply(client->getFd(), joinMsg);
  }

  return result;
}


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
    // result.addReply(fd, ReplyBuilder::alreadyRegistered(*client));
    result.addReply(fd, ReplyBuilder::noRegistered(*client));
    return result;
  }
  if (!msg.hasParam(0)) {
    result.addReply(fd, ReplyBuilder::needMoreParams(client, "PART"));
    return result;
  }

  const std::string channelName = msg.getSingleParam(0);
  Channel* channel = state.getChannel(channelName);
  state.removeClientFromChannel(client, channelName);
  std::string partMsg = ReplyBuilder::join(channelName, client->getFullPrefix(), "PART");
  std::vector<Client*> members = channel->getMembers();	// ディープコピーじゃなくていいのかな？
  for (std::vector<Client*>::iterator it = members.begin(); it != members.end(); ++it) {
      Client * client = *it;
	  result.addReply(client->getFd(), partMsg);
  }
  return result;
}


CommandResult CommandDispatcher::handlePrivmsg(int fd, const Message& msg,
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
    result.addReply(fd, ReplyBuilder::needMoreParams(client, "PRIVMSG"));
    return result;
  }

  const std::string& targetName = msg.getSingleParam(0);
  const std::string& text   = msg.getSingleParam(1);

  std::string privmsgMsg = ReplyBuilder::privmsg(client->getFullPrefix(), targetName, text);

  if (targetName.empty() || targetName[0] == '#') {
      Channel* channel = state.getChannel(targetName);
      if (channel == NULL) {
      result.addReply(fd, ReplyBuilder::noSuchChannel(*client, targetName));
      return result;
      }
      if (!channel->hasMember(client)) {
      result.addReply(fd, ReplyBuilder::cannotSendToChan(*client, targetName));
      return result;
      }
    std::vector<Client*> members = channel->getMembers();    // ディープコピーじゃなくていいのかな？
    for (std::vector<Client*>::iterator it = members.begin(); it != members.end(); ++it) {
      Client* member = *it;
      if (member->getFd() == fd) {
        result.addReply(member->getFd(), privmsgMsg);
      }
    }
  } else {
      Client* targetClient = state.getClientByNick(targetName);
    if (targetClient == NULL) {
        result.addReply(fd, ReplyBuilder::noSuchNick(*client, targetName));
        return result;
    }
      result.addReply(targetClient->getFd(), privmsgMsg);
  }
  return result;
}


// CommandResult CommandDispatcher::handlePrivmsg(int fd, const Message& msg,
//                                             ServerState& state,
//                                             Client* client) {
//   CommandResult result;
//   if (!client) {
//     return result;
//   }
// //   if (!client->isPassOk()) {
// //     result.addReply(fd, ReplyBuilder::passwordMismatch());
// //     return result;
// //   }
//   if (!client->isRegistered()) {
//     // result.addReply(fd, ReplyBuilder::alreadyRegistered(*client));
//     result.addReply(fd, ReplyBuilder::noRegistered(*client));
//     return result;
//   }
//   if (msg.hasParam(0)) {
//     result.addReply(fd, ReplyBuilder::needMoreParams(client, "PRIVMSG"));
//     return result;
//   }

//   const std::string receiverName = msg.getSingleParam(0);
//   const std::vector<std::string>& textToSend = msg.getParams();

//   const std::string channelName = msg.getSingleParam(0);
//   Channel* channel = state.getChannel(channelName);
//   state.removeClientFromChannel(client, channelName);
//   std::string partMsg = ReplyBuilder::join(channelName, client->getFullPrefix(), "PRIVMSG");
//   std::vector<Client*> members = channel->getMembers();	// ディープコピーじゃなくていいのかな？
//   for (std::vector<Client*>::iterator it = members.begin(); it != members.end(); ++it) {
//       Client * client = *it;
// 	  result.addReply(client->getFd(), partMsg);
//   }
//   return result;
// }


// "NOTICE"
// CommandResult CommandDispatcher::handleNotice(int fd, const Message& msg,
//                                             ServerState& state,
//                                             Client* client) {

// }


// "QUIT"
// CommandResult CommandDispatcher::handleQuit(int fd, const Message& msg,
//                                             ServerState& state,
//                                             Client* client) {

// }


// "KICK"
// CommandResult CommandDispatcher::handleKick(int fd, const Message& msg,
//                                             ServerState& state,
//                                             Client* client) {

// }


// "INVITE"
// CommandResult CommandDispatcher::handleInvite(int fd, const Message& msg,
//                                             ServerState& state,
//                                             Client* client) {

// }


// "TOPIC"
// CommandResult CommandDispatcher::handleTopic(int fd, const Message& msg,
//                                             ServerState& state,
//                                             Client* client) {

// }


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
