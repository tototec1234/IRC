#include "irc/LineBuffer.hpp"

namespace bonus {
namespace irc {

LineBuffer::LineBuffer() {}

LineBuffer::~LineBuffer() {}

void LineBuffer::append(const char* data, size_t size) {
  _buffer.append(data, size);

  size_t lineEnd = _buffer.find('\n');
  while (lineEnd != std::string::npos) {
    std::string line = _buffer.substr(0, lineEnd);
    if (!line.empty() && line[line.size() - 1] == '\r') {
      line.erase(line.size() - 1);
    }
    _lines.push(line);
    _buffer.erase(0, lineEnd + 1);
    lineEnd = _buffer.find('\n');
  }
}

bool LineBuffer::hasLine() const { return !_lines.empty(); }

std::string LineBuffer::popLine() {
  if (_lines.empty()) {
    return std::string();
  }
  std::string line = _lines.front();
  _lines.pop();
  return line;
}

}  // namespace irc
}  // namespace bonus
