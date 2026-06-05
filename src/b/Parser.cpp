#include "Parser.hpp"

#include <cctype>
#include <string>
#include <vector>

namespace {

std::string removeIrcLineTerminator(const std::string& line) {
  if (line.size() >= 2 && line[line.size() - 2] == '\r' &&
      line[line.size() - 1] == '\n') {
    return line.substr(0, line.size() - 2);
  }
  return line;
}

std::string upperCommand(const std::string& command) {
  std::string upper = command;
  for (size_t i = 0; i < upper.size(); ++i) {
    upper[i] =
        static_cast<char>(std::toupper(static_cast<unsigned char>(upper[i])));
  }
  return upper;
}

bool isMessageSeparator(char c) { return c == ' '; }

void skipSeparators(const std::string& input, size_t& pos) {
  while (pos < input.size() && isMessageSeparator(input[pos])) {
    ++pos;
  }
}

bool skipPrefix(const std::string& input, size_t& pos) {
  skipSeparators(input, pos);
  if (pos >= input.size() || input[pos] != ':') {
    return true;
  }

  size_t prefixEnd = pos;
  while (prefixEnd < input.size() && !isMessageSeparator(input[prefixEnd])) {
    ++prefixEnd;
  }
  if (prefixEnd == input.size()) {
    return false;
  }
  pos = prefixEnd;
  skipSeparators(input, pos);
  return true;
}

std::string readToken(const std::string& input, size_t& pos) {
  size_t tokenEnd = pos;
  while (tokenEnd < input.size() && !isMessageSeparator(input[tokenEnd])) {
    ++tokenEnd;
  }
  std::string token = input.substr(pos, tokenEnd - pos);
  pos = tokenEnd;
  return token;
}

std::vector<std::string> readParams(const std::string& input, size_t pos) {
  std::vector<std::string> params;

  while (pos < input.size()) {
    skipSeparators(input, pos);
    if (pos >= input.size()) {
      break;
    }
    if (input[pos] == ':') {
      params.push_back(input.substr(pos + 1));
      break;
    }
    params.push_back(readToken(input, pos));
  }
  return params;
}

}  // namespace

Message Parser::parse(const std::string& line) {
  std::string input = removeIrcLineTerminator(line);
  size_t pos = 0;

  if (!skipPrefix(input, pos) || pos >= input.size()) {
    return Message();
  }

  std::string command = readToken(input, pos);
  std::vector<std::string> params = readParams(input, pos);
  return Message(upperCommand(command), params);
}
