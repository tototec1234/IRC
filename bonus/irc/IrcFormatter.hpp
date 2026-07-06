#ifndef BONUS_IRC_IRCFORMATTER_HPP
#define BONUS_IRC_IRCFORMATTER_HPP

#include <string>

namespace bonus {
namespace irc {

class IrcFormatter {
 public:
  static std::string pass(const std::string& password);
  static std::string nick(const std::string& nickname);
  static std::string user(const std::string& username,
                          const std::string& realname);
  static std::string pong(const std::string& token);
  static std::string join(const std::string& channel);
  static std::string privmsg(const std::string& target,
                             const std::string& text);
  static std::string quit(const std::string& message);

 private:
  static std::string commandWithTrailing(const std::string& command,
                                         const std::string& target,
                                         const std::string& trailing);

  IrcFormatter();
  IrcFormatter(const IrcFormatter&);
  IrcFormatter& operator=(const IrcFormatter&);
};

}  // namespace irc
}  // namespace bonus

#endif
