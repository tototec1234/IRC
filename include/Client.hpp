#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client {
 public:
  Client();
  ~Client();

 private:
  std::string _nickname;
  std::string _username;
};

#endif
