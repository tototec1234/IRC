#include "CommandResult.hpp"

OutgoingMessage::OutgoingMessage(int targetFd, const std::string& text)
    : fd(targetFd), message(text) {}

CommandResult::CommandResult() : shouldDisconnect(false) {}

void CommandResult::addReply(int fd, const std::string& message) {
  replies.push_back(OutgoingMessage(fd, message));
}
