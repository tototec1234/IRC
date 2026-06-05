#include "ReplyBuilder.hpp"

#include "Client.hpp"

namespace {

const char* SERVER_NAME = "irc.local";

const char* RPL_WELCOME = "001";  // welcome()
// TODO: const char* RPL_CHANNELMODEIS = "324";  // channelModeIs()
// TODO: const char* RPL_NOTOPIC = "331";        // noTopic()
// TODO: const char* RPL_TOPIC = "332";          // topicReply()
// TODO: const char* RPL_INVITING = "341";       // inviting()
// TODO: const char* RPL_NAMREPLY = "353";       // nameReply()
// TODO: const char* RPL_ENDOFNAMES = "366";     // endOfNames()

// TODO: const char* ERR_NOSUCHNICK = "401";       // noSuchNick()
// TODO: const char* ERR_NOSUCHCHANNEL = "403";    // noSuchChannel()
// TODO: const char* ERR_CANNOTSENDTOCHAN = "404"; // cannotSendToChan()
const char* ERR_UNKNOWNCOMMAND = "421";  // unknownCommand()
// TODO: const char* ERR_NONICKNAMEGIVEN = "431";  // noNicknameGiven()
const char* ERR_NICKNAMEINUSE = "433";  // nickInUse()
// TODO: const char* ERR_USERNOTINCHANNEL = "441"; // userNotInChannel()
// TODO: const char* ERR_NOTONCHANNEL = "442";     // notOnChannel()
// TODO: const char* ERR_NOTREGISTERED = "451";    // notRegistered()
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

std::string ReplyBuilder::unknownCommand(const Client* client,
                                         const std::string& command) {
  return numericReply(ERR_UNKNOWNCOMMAND, replyTarget(client), command,
                      "Unknown command");
}
