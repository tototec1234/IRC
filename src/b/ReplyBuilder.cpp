#include "b/ReplyBuilder.hpp"

#include "c/Client.hpp"

namespace {

const char* SERVER_NAME = "irc.local";

const char* RPL_WELCOME = "001";  // welcome()
// TODO: const char* RPL_CHANNELMODEIS = "324";  // channelModeIs()
// TODO: const char* RPL_NOTOPIC = "331";        // noTopic()
// TODO: const char* RPL_TOPIC = "332";          // topicReply()
// TODO: const char* RPL_INVITING = "341";       // inviting()
// TODO: const char* RPL_NAMREPLY = "353";       // nameReply()
// TODO: const char* RPL_ENDOFNAMES = "366";     // endOfNames()

const char* ERR_NOSUCHNICK = "401";       // noSuchNick()
const char* ERR_NOSUCHCHANNEL = "403";    // noSuchChannel()
const char* ERR_CANNOTSENDTOCHAN = "404"; // cannotSendToChan()
const char* ERR_UNKNOWNCOMMAND = "421";  // unknownCommand()
// TODO: const char* ERR_NONICKNAMEGIVEN = "431";  // noNicknameGiven()
const char* ERR_NICKNAMEINUSE = "433";  // nickInUse()
const char* ERR_USERNOTINCHANNEL = "441"; // userNotInChannel()
const char* ERR_NOTONCHANNEL = "442";     // notOnChannel()
const char* ERR_NOTREGISTERED = "451";    // notRegistered()
const char* ERR_NEEDMOREPARAMS = "461";     // needMoreParams()
const char* ERR_ALREADYREGISTERED = "462";  // alreadyRegistered()
const char* ERR_PASSWDMISMATCH = "464";     // passwordMismatch()
// TODO: const char* ERR_CHANNELISFULL = "471";    // channelIsFull()
// TODO: const char* ERR_INVITEONLYCHAN = "473";   // inviteOnlyChan()
// TODO: const char* ERR_BADCHANNELKEY = "475";    // badChannelKey()
// TODO: const char* ERR_CHANOPRIVSNEEDED = "482"; // chanOpPrivsNeeded()

std::string replyTarget(const Client* client) {
  if (!client || client->getNick().empty()) {
    return "*";
  }
  return client->getNick();
}

std::string numericReply(const std::string& code, const std::string& target,
                         const std::string& params, const std::string& text) {
  std::string reply =
      std::string(":") + SERVER_NAME + " " + code + " " + target;
  if (!params.empty()) {
    reply += " " + params;
  }
  reply += " :" + text + "\r\n";
  return reply;
}

}  // namespace

std::string ReplyBuilder::pong(const std::string& token) {
  return std::string(":") + SERVER_NAME + " PONG " + SERVER_NAME + " :" +
         token + "\r\n";
}

std::string ReplyBuilder::welcome(const Client& client) {
  return numericReply(RPL_WELCOME, client.getNick(), "",
                      "Welcome to the IRC Network " + client.getFullPrefix());
}

std::string ReplyBuilder::needMoreParams(const Client* client,
                                         const std::string& command) {
  return numericReply(ERR_NEEDMOREPARAMS, replyTarget(client), command,
                      "Not enough parameters");
}

std::string ReplyBuilder::alreadyRegistered(const Client& client) {
  return numericReply(ERR_ALREADYREGISTERED, replyTarget(&client), "",
                      "You may not reregister");
}

std::string ReplyBuilder::passwordMismatch() {
  return numericReply(ERR_PASSWDMISMATCH, "*", "", "Password incorrect");
}

std::string ReplyBuilder::nickInUse(const std::string& nick) {
  return numericReply(ERR_NICKNAMEINUSE, "*", nick,
                      "Nickname is already in use");
}



std::string ReplyBuilder::noSuchNick(const Client& client,
                                     const std::string& targetName) {
  return numericReply(ERR_NOSUCHNICK, replyTarget(&client), targetName,
                      "No such nick/channel");
}

std::string ReplyBuilder::noSuchChannel(const Client& client,
                                        const std::string& ChannelName) {
  return numericReply(ERR_NOSUCHCHANNEL, replyTarget(&client), ChannelName,
                      "No such channel");
}

//                               //
std::string ReplyBuilder::cannotSendToChan(const Client& client,
                                           const std::string& ChannelName) {
return numericReply(ERR_CANNOTSENDTOCHAN, replyTarget(&client), ChannelName,
                    "Cannot send to channel");
}

std::string ReplyBuilder::userNotInChannel(const Client& client,
                                           const std::string& ChannelName) {
  return numericReply(ERR_USERNOTINCHANNEL, replyTarget(&client), ChannelName,
                      "You are not on that channel");
}

std::string ReplyBuilder::notOnChannel(const Client& client,
                                       const std::string& ChannelName) {
  return numericReply(ERR_NOTONCHANNEL, replyTarget(&client), ChannelName,
                      "You are not on that channel");
}

std::string ReplyBuilder::noRegistered(const Client& client) {
  return numericReply(ERR_NOTREGISTERED, replyTarget(&client), "",
                      ":You have not registered");
}



std::string ReplyBuilder::join(const std::string & ChannelName,
							   const std::string & clientFullPrefix,
                               const std::string& command) {
  return std::string(":") + clientFullPrefix + " " + command +
  		 " " + ChannelName + "\r\n";
}

std::string ReplyBuilder::privmsg(const std::string& client,
                                  const std::string& target,
                                  const std::string& text) {
  return std::string(":") + client + " PRIVMSG " + target + " :" + text + "\r\n";
}



std::string ReplyBuilder::unknownCommand(const Client* client,
                                         const std::string& command) {
  return numericReply(ERR_UNKNOWNCOMMAND, replyTarget(client), command,
                      "Unknown command");
}




//	torima ato de kesu
std::string ReplyBuilder::torima_Missing(const Client& client,
                                         const std::string& command) {
  return numericReply("999", replyTarget(&client), command,
	                  "みすった〜");
}