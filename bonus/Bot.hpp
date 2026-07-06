#ifndef BONUS_BOT_HPP
#define BONUS_BOT_HPP

#include <queue>
#include <string>

#include "b/Message.hpp"
#include "irc/LineBuffer.hpp"

namespace bonus {

class Bot {
 public:
  Bot(const std::string& host, const std::string& port,
      const std::string& password, const std::string& channel,
      const std::string& nickname);
  ~Bot();

  int run();

 private:
  std::string _host;
  std::string _port;
  std::string _password;
  std::string _channel;
  std::string _nickname;
  int _fd;
  bool _running;
  bool _joined;
  irc::LineBuffer _lineBuffer;
  std::queue<std::string> _outbox;

  void connectToServer();
  void enqueueRegistration();
  void enqueue(const std::string& line);
  void readFromServer();
  void flushOutput();
  void handleMessage(const Message& message);
  void handlePrivmsg(const Message& message);
  void sendReply(const Message& message, const std::string& text);
  std::string replyTarget(const Message& message) const;
  std::string senderNick(const std::string& prefix) const;
  std::string buildCommandReply(const std::string& text);

  Bot();
  Bot(const Bot&);
  Bot& operator=(const Bot&);
};

}  // namespace bonus

#endif
