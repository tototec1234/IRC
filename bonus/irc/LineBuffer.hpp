#ifndef BONUS_IRC_LINEBUFFER_HPP
#define BONUS_IRC_LINEBUFFER_HPP

#include <queue>
#include <string>

namespace bonus {
namespace irc {

class LineBuffer {
 public:
  LineBuffer();
  ~LineBuffer();

  void append(const char* data, size_t size);
  bool hasLine() const;
  std::string popLine();

 private:
  std::string _buffer;
  std::queue<std::string> _lines;

  LineBuffer(const LineBuffer&);
  LineBuffer& operator=(const LineBuffer&);
};

}  // namespace irc
}  // namespace bonus

#endif
