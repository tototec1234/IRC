#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <set>
#include <string>

#include "Client.hpp"

/*
  設計では、ChannelModeをクラスとして定義しているが、
  単なる状態にすぎないのでEnumや構造体でメンバとして保持するほうがシンプルではないか
*/

class Channel {
 public:
  Channel(const std::string& name);
  ~Channel();

  const std::string& getName() const;
  bool hasClient(const Client* client) const;
  void addClient(Client* client);
  void removeClient(Client* client);
  bool isEmpty() const;

 private:
  std::string _name;
  std::set<Client*> _clients;

  Channel();
  Channel(const Channel&);
  Channel& operator=(const Channel&);
};

#endif
