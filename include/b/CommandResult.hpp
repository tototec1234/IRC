#ifndef COMMANDRESULT_HPP
#define COMMANDRESULT_HPP

#include <string>
#include <vector>

struct OutgoingMessage {
  // Target fd that A layer should enqueue this message to.
  int fd;
  // Complete IRC reply/notification text. B layer builds the string but does
  // not send it directly.
  std::string message;

  OutgoingMessage(int targetFd, const std::string& text);
};

struct CommandResult {
  // Replies and broadcasts produced by one command.
  std::vector<OutgoingMessage> replies;
  /*
   * @brief Requests A layer to disconnect the source fd after applying replies.
   *
   * B layer never closes sockets directly. QUIT and future fatal protocol
   * decisions should set this flag and let A layer perform the actual cleanup.
   */
  bool shouldDisconnect;

  CommandResult();
  /*
   * @brief Adds one outgoing IRC message to the result.
   *
   * The message should already include the IRC line terminator.
   */
  void addReply(int fd, const std::string& message);
};

#endif