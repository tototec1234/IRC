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
  } else if (command == "NICK") {
    result = handleNick(fd, msg, state, client);
  } else if (command == "USER") {
    result = handleUser(fd, msg, client);
  } else if (command == "JOIN") {
	result = handleJoin(fd, msg, state, client);
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
  if (!client->isPassOk()) {
    result.addReply(fd, ReplyBuilder::passwordMismatch());
    return result;
  }
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
  if (!client->isPassOk()) {
    result.addReply(fd, ReplyBuilder::passwordMismatch());
    return result;
  }
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
  if (!msg.hasParam(0)) {
    result.addReply(fd, ReplyBuilder::needMoreParams(client, "JOIN"));
    return result;
  }
//   if (!client->isPassOk()) {
  if (!client->isRegistered()) {
    // result.addReply(fd, ReplyBuilder::alreadyRegistered(*client));
    result.addReply(fd, ReplyBuilder::noRegistered(*client));
    return result;
  }
  Channel *channel = state.addClientToChannel(client, msg.getSingleParam(0));
  if (!channel) {
	result.addReply(fd, ReplyBuilder::torima_joinMissing(*client, "JOIN"));
  	return result;
  }

  std::string joinMsg = ReplyBuilder::join(channel->getName(), client->getFullPrefix(), "JOIN");
  
  std::vector<Client*> members = channel->getMembers();	// ディープコピーじゃなくていいのかな？

  for (std::vector<Client*>::iterator it = members.begin(); it != members.end(); ++it) {
      Client * client = *it;
	  result.addReply(client->getFd(), joinMsg);
  }

//   result.addReply(fd, ReplyBuilder::join(channel->getName(), "JOIN"));	//Channelクラスのgetterを勝手に触っていいのだろうか。。。？
//   ReplyBuilder::join(msg.getSingleParam(0), "JOIN");
  return result;
}

void CommandDispatcher::maybeRegister(Client& client, CommandResult& result) {
  if (!client.isRegistered() && client.canRegister()) {
    client.markRegistered();
    result.addReply(client.getFd(), ReplyBuilder::welcome(client));
  }
}
