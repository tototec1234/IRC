#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>

#include "Message.hpp"

class Parser {
 public:
  /**
   * @brief Converts one complete IRC line into Message.
   * Parser expects one complete IRC message. The message may include the final
   * CRLF because A layer extracts message boundaries but does not normalize IRC
   * syntax for B layer.
   */
  static Message parse(const std::string& line);

 private:
  Parser();
  Parser(const Parser&);
  Parser& operator=(const Parser&);
};

#endif
