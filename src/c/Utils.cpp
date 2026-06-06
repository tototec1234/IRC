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
