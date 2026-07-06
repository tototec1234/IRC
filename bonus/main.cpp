#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>

#include "Bot.hpp"

namespace {

bool isValidPort(const std::string& port) {
  if (port.empty()) {
    return false;
  }
  for (size_t i = 0; i < port.size(); ++i) {
    if (port[i] < '0' || port[i] > '9') {
      return false;
    }
  }
  char* end = NULL;
  long value = std::strtol(port.c_str(), &end, 10);
  return end && *end == '\0' && value > 0 && value <= 65535;
}

bool isValidChannel(const std::string& channel) {
  return !channel.empty() && (channel[0] == '#');
}

void printUsage(const char* program) {
  std::cerr << "Usage: " << program
            << " <host> <port> <password> <channel> [nickname]" << std::endl;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc != 5 && argc != 6) {
    printUsage(argv[0]);
    return 1;
  }

  std::string host = argv[1];
  std::string port = argv[2];
  std::string password = argv[3];
  std::string channel = argv[4];
  std::string nickname = argc == 6 ? argv[5] : "bonusbot";

  if (!isValidPort(port)) {
    std::cerr << "Error: invalid port '" << port << "'" << std::endl;
    return 1;
  }
  if (!isValidChannel(channel)) {
    std::cerr << "Error: channel must start with # or &" << std::endl;
    return 1;
  }
  if (nickname.empty()) {
    std::cerr << "Error: nickname must not be empty" << std::endl;
    return 1;
  }

  try {
    bonus::Bot bot(host, port, password, channel, nickname);
    return bot.run();
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
