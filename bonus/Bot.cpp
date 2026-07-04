
#include "Bot.hpp"

#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netdb.h>
#include <poll.h>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "b/Parser.hpp"
#include "irc/IrcFormatter.hpp"

namespace {

const int POLL_TIMEOUT_MS = 1000;
const size_t READ_BUFFER_SIZE = 4096;

void setNonBlocking(int fd) {
  if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0) {
    throw std::runtime_error("fcntl(O_NONBLOCK) failed");
  }
}

void closeIfOpen(int& fd) {
  if (fd >= 0) {
    close(fd);
    fd = -1;
  }
}

std::string currentTimeText() {
  std::time_t now = std::time(NULL);
  std::tm* timeInfo = std::localtime(&now);
  char buffer[64];
  if (!timeInfo ||
      std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeInfo) ==
          0) {
    return "time unavailable";
  }
  return std::string(buffer);
}

}  // namespace

namespace bonus {

Bot::Bot(const std::string& host, const std::string& port,
         const std::string& password, const std::string& channel,
         const std::string& nickname)
    : _host(host),
      _port(port),
      _password(password),
      _channel(channel),
      _nickname(nickname),
      _fd(-1),
      _running(true),
      _joined(false) {}

Bot::~Bot() { closeIfOpen(_fd); }

int Bot::run() {
  signal(SIGPIPE, SIG_IGN);
  connectToServer();
  enqueueRegistration();

  while (_running) {
    struct pollfd pfd;
    pfd.fd = _fd;
    pfd.events = POLLIN;
    if (!_outbox.empty()) {
      pfd.events |= POLLOUT;
    }
    pfd.revents = 0;

    int ready = poll(&pfd, 1, POLL_TIMEOUT_MS);
    if (ready < 0) {
      throw std::runtime_error("poll failed");
    }
    if (ready == 0) {
      continue;
    }
    if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) {
      throw std::runtime_error("server connection closed");
    }
    if (pfd.revents & POLLIN) {
      readFromServer();
    }
    if (pfd.revents & POLLOUT) {
      flushOutput();
    }
  }

  while (!_outbox.empty()) {
    flushOutput();
  }
  return 0;
}

void Bot::connectToServer() {
  struct addrinfo hints;
  struct addrinfo* result = NULL;
  struct addrinfo* it = NULL;

  std::memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  int status = getaddrinfo(_host.c_str(), _port.c_str(), &hints, &result);
  if (status != 0) {
    throw std::runtime_error("getaddrinfo failed");
  }

  for (it = result; it != NULL; it = it->ai_next) {
    _fd = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
    if (_fd < 0) {
      continue;
    }
    if (connect(_fd, it->ai_addr, it->ai_addrlen) == 0) {
      break;
    }
    closeIfOpen(_fd);
  }
  freeaddrinfo(result);

  if (_fd < 0) {
    throw std::runtime_error("could not connect to IRC server");
  }
  setNonBlocking(_fd);
}

void Bot::enqueueRegistration() {
  if (!_password.empty()) {
    enqueue(irc::IrcFormatter::pass(_password));
  }
  enqueue(irc::IrcFormatter::nick(_nickname));
  enqueue(irc::IrcFormatter::user(_nickname, "ft_irc bonus bot"));
}

void Bot::enqueue(const std::string& line) { _outbox.push(line); }

void Bot::readFromServer() {
  char buffer[READ_BUFFER_SIZE];
  ssize_t received = recv(_fd, buffer, sizeof(buffer), 0);
  if (received < 0) {
    throw std::runtime_error("recv failed");
  }
  if (received == 0) {
    throw std::runtime_error("server closed connection");
  }

  _lineBuffer.append(buffer, static_cast<size_t>(received));
  while (_lineBuffer.hasLine()) {
    handleMessage(Parser::parse(_lineBuffer.popLine()));
  }
}

void Bot::flushOutput() {
  while (!_outbox.empty()) {
    std::string& line = _outbox.front();
    ssize_t sent = send(_fd, line.data(), line.size(), 0);
    if (sent <= 0) {
      throw std::runtime_error("send failed");
    }
    line.erase(0, static_cast<size_t>(sent));
    if (!line.empty()) {
      return;
    }
    _outbox.pop();
  }
}

void Bot::handleMessage(const Message& message) {
  if (message.getCommand() == "PING" && message.hasParam(0)) {
    enqueue(irc::IrcFormatter::pong(message.getSingleParam(0)));
    return;
  }
  if (message.getCommand() == "001" && !_joined) {
    enqueue(irc::IrcFormatter::join(_channel));
    _joined = true;
    return;
  }
  if (message.getCommand() == "PRIVMSG") {
    handlePrivmsg(message);
  }
}

void Bot::handlePrivmsg(const Message& message) {
  if (!message.hasParam(1)) {
    return;
  }
  const std::string& text = message.getSingleParam(1);
  if (text.empty() || text[0] != '!') {
    return;
  }
  sendReply(message, buildCommandReply(text));
}

void Bot::sendReply(const Message& message, const std::string& text) {
  if (!text.empty()) {
    enqueue(irc::IrcFormatter::privmsg(replyTarget(message), text));
  }
}

std::string Bot::replyTarget(const Message& message) const {
  const std::string& target = message.getSingleParam(0);
  if (!target.empty() && (target[0] == '#' || target[0] == '&')) {
    return target;
  }
  return senderNick(message.getPrefix());
}

std::string Bot::senderNick(const std::string& prefix) const {
  size_t end = prefix.find('!');
  if (end == std::string::npos) {
    end = prefix.find('@');
  }
  if (end == std::string::npos) {
    return prefix;
  }
  return prefix.substr(0, end);
}

std::string Bot::buildCommandReply(const std::string& text) {
  if (text == "!help") {
    return "commands: !help !ping !echo <text> !time !quit";
  }
  if (text == "!ping") {
    return "pong";
  }
  if (text.find("!echo ") == 0) {
    return text.substr(6);
  }
  if (text == "!time") {
    return currentTimeText();
  }
  if (text == "!quit") {
    enqueue(irc::IrcFormatter::quit("bonus bot shutting down"));
    _running = false;
    return "bye";
  }
  return "unknown command; try !help";
}

}  // namespace bonus
