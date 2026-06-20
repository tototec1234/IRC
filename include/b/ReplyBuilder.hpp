#ifndef REPLYBUILDER_HPP
#define REPLYBUILDER_HPP

#include <string>

class Client;

class ReplyBuilder {
 public:
  // Non-numeric connection keepalive reply.
  static std::string ping(const std::string& token);
  static std::string pong(const std::string& token);

  // Numeric replies currently used by CommandDispatcher skeleton.
  static std::string welcome(const Client& client);
  static std::string needMoreParams(const Client& client,
                                    const std::string& command);
  static std::string alreadyRegistered(const Client& client);
  static std::string passwordMismatch();
  static std::string nickInUse(const std::string& nick);
  static std::string noSuchNick(const Client& client,
         		                const std::string& targetName);
  static std::string noSuchChannel(const Client& client,
                                   const std::string& ChannelName);
  static std::string noTopic(const Client& client,
                             const std::string& ChannelName);
  static std::string topicReply(const Client& client,
                                const std::string& ChannelName,
                                const std::string& topic);
  static std::string inviting(const Client& client,
                              const std::string& targetNick,
                              const std::string& ChannelName);
  static std::string cannotSendToChan(const Client& client,
                                      const std::string& ChannelName);
  static std::string userNotInChannel(const Client& client,
                                      const std::string& ChannelName);
  static std::string notOnChannel(const Client& client,
                                  const std::string& ChannelName);
  static std::string userOnChannel(const Client& client,
                                   const std::string& targetNick,
                                   const std::string& ChannelName);
  static std::string chanOpPrivsNeeded(const Client& client,
                                       const std::string& ChannelName);
  static std::string inviteOnlyChan(const Client& client,
                                    const std::string& ChannelName);
  static std::string noRegistered(const Client& client);
  static std::string unknownCommand(const Client& client,
                                    const std::string& command);
  static std::string unknownMode(const Client& client,
                                 const std::string& modeToken);



  static std::string join(const std::string& clientFullPrefix,
                          const std::string& command,
						  const std::string& ChannelName);
  static std::string part(const std::string& clientFullPrefix,
                          const std::string& command,
						  const std::string& ChannelName);
  static std::string privmsg(const std::string& client,
                             const std::string& command,
                             const std::string& target,
                             const std::string& text);
  static std::string notice(const std::string& client,
                            const std::string& command,
                            const std::string& target,
                            const std::string& text);
  static std::string topic(const std::string& client,
                           const std::string& command,
                           const std::string& target,
                           const std::string& text);
  static std::string invite(const std::string& client,
                            const std::string& target,
                            const std::string& ChannelName);
  static std::string kick(const std::string& client,
                          const std::string& ChannelName,
                          const std::string& target,
                          const std::string& reason);
  static std::string mode(const std::string& client,
                          const std::string& ChannelName,
                          const std::string& modeToken,
                          const std::string& modeArg);
  static std::string quit(const std::string& client,
                          const std::string& reason);



  static std::string torima_Missing(const Client& client,
                                         const std::string& command);

 private:
  // Static utility class.
  ReplyBuilder();
  ReplyBuilder(const ReplyBuilder&);
  ReplyBuilder& operator=(const ReplyBuilder&);
};

#endif
