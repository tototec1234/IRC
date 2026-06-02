#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>

/*
RFC1459に従った大文字小文字の比較を行うための比較関数オブジェクト
この比較関数は、std::mapやstd::setなどの連想コンテナで使用されることを想定している
7bitのASCII文字を前提とし、8bitの文字はパース層でエラーとする
*/
struct IrcStringCompare {
  bool operator()(const std::string& a, const std::string& b) const;
};

#endif
