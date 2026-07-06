#include "irc/IrcFormatter.hpp"

namespace bonus {
namespace irc {

std::string IrcFormatter::pass(const std::string& password) {
  return "PASS " + password + "\r\n";
}

std::string IrcFormatter::nick(const std::string& nickname) {
  return "NICK " + nickname + "\r\n";
}

std::string IrcFormatter::user(const std::string& username,
                               const std::string& realname) {
  return "USER " + username + " 0 * :" + realname + "\r\n";
}

std::string IrcFormatter::pong(const std::string& token) {
  return "PONG :" + token + "\r\n";
}

std::string IrcFormatter::join(const std::string& channel) {
  return "JOIN " + channel + "\r\n";
}

std::string IrcFormatter::privmsg(const std::string& target,
                                  const std::string& text) {
  return commandWithTrailing("PRIVMSG", target, text);
}

std::string IrcFormatter::quit(const std::string& message) {
  return "QUIT :" + message + "\r\n";
}

std::string IrcFormatter::commandWithTrailing(const std::string& command,
                                              const std::string& target,
                                              const std::string& trailing) {
  return command + " " + target + " :" + trailing + "\r\n";
}

}  // namespace irc
}  // namespace bonus
