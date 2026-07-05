#include "c/Utils.hpp"

#include <algorithm>
#include <cctype>
#include <string>

namespace {
char irc_tolower(char c) {
  if (c >= 'A' && c <= 'Z') return c + ('a' - 'A');
  if (c == '[') return '{';
  if (c == ']') return '}';
  if (c == '\\') return '|';
  if (c == '~') return '^';
  return c;
}

bool isLetter(char c) {
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

bool isDigit(char c) { return c >= '0' && c <= '9'; }

bool isSpecial(char c) {
  return (c >= '[' && c <= '`') || (c >= '{' && c <= '}');
}

bool isNicknameChar(char c) {
  return isLetter(c) || isDigit(c) || isSpecial(c) || c == '-';
}

bool isChannelStringChar(char c) {
  unsigned char value = static_cast<unsigned char>(c);
  if (value == '\0' || value == '\a' || value == '\r' || value == '\n') {
    return false;
  }
  return c != ' ' && c != ',' && c != ':';
}
};  // namespace

bool IrcStringCompare::operator()(const std::string& a,
                                  const std::string& b) const {
  size_t len = std::min(a.size(), b.size());
  for (size_t i = 0; i < len; ++i) {
    char lower_a = irc_tolower(a[i]);
    char lower_b = irc_tolower(b[i]);
    if (lower_a < lower_b) return true;
    if (lower_a > lower_b) return false;
  }
  return a.size() < b.size();
}

bool isValidNickname(const std::string& name) {
  if (name.empty() || name.size() > 9) {
    return false;
  }
  if (!isLetter(name[0]) && !isSpecial(name[0])) {
    return false;
  }
  for (size_t i = 1; i < name.size(); ++i) {
    if (!isNicknameChar(name[i])) {
      return false;
    }
  }
  return true;
}

bool isValidChannelName(const std::string& name) {
  if (name.size() < 2 || name[0] != '#') {
    return false;
  }
  for (size_t i = 1; i < name.size(); ++i) {
    if (!isChannelStringChar(name[i])) {
      return false;
    }
  }
  return true;
}
