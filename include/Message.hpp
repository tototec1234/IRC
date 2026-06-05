#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <vector>

class Message {
 public:
  /**
   * @brief Creates an empty message.
   *
   * Used as a sentinel for lines that Parser cannot turn into a command.
   * CommandDispatcher treats an empty command as no-op.
   */
  Message();
  /**
   * @brief Creates a parsed IRC message.
   *
   * command is expected to be normalized by Parser. params stores middle
   * parameters and the optional trailing parameter as plain strings.
   */
  Message(const std::string& command, const std::vector<std::string>& params);
  ~Message();

  const std::string& getCommand() const;
  const std::vector<std::string>& getParams() const;
  size_t getParamCount() const;
  /**
   * @brief Returns the parameter at index, or an empty string if out of range.
   *
   * Prefer hasParam() when the caller must distinguish missing from empty.
   */
  const std::string& getSingleParam(size_t index) const;
  bool hasParam(size_t index) const;

 private:
  // IRC command name. Parser currently normalizes command names to uppercase.
  std::string _command;
  // Parsed IRC parameters.
  // If the input has a trailing parameter introduced by ':', Parser removes
  // that marker and stores the rest of the line as one final parameter,
  // preserving spaces inside it.
  // Example: "PRIVMSG #room :hello world" -> ["#room", "hello world"]
  std::vector<std::string> _params;
};

#endif
