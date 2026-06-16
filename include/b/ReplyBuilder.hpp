#ifndef REPLYBUILDER_HPP
#define REPLYBUILDER_HPP

#include <string>

class Client;

class ReplyBuilder {
 public:
  // Non-numeric connection keepalive reply.
  static std::string pong(const std::string& token);

  // Numeric replies currently used by CommandDispatcher skeleton.
  static std::string welcome(const Client& client);
  static std::string needMoreParams(const Client* client,
                                    const std::string& command);
  static std::string alreadyRegistered(const Client& client);
  static std::string passwordMismatch();
  static std::string nickInUse(const std::string& nick);
  static std::string noRegistered(const Client& client);
  static std::string unknownCommand(const Client* client,
                                    const std::string& command);



  static std::string join(const std::string & ChannelName,
                          const std::string& command);



  static std::string torima_joinMissing(const Client& client,
                                         const std::string& command);

 private:
  // Static utility class.
  ReplyBuilder();
  ReplyBuilder(const ReplyBuilder&);
  ReplyBuilder& operator=(const ReplyBuilder&);
};

#endif
