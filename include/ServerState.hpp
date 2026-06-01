#ifndef SERVERSTATE_HPP
#define SERVERSTATE_HPP

#include <map>
#include <string>

#include "Channel.hpp"
#include "Client.hpp"
#include "ClientRegistry.hpp"

class ServerState {
 public:
  ServerState();
  ~ServerState();

  const std::string& getPassword() const;
  void addclient(int fd);
  /*
    client削除によりchannelの参加者が0になった場合はchannelも削除する
    その場合、clientは自分の参加しているチャンネルを持つ必要がある
    フロー：
      client取得
      clientから参加しているチャンネルのリストを取得
      参加しているチャンネルのリストをループ
      チャンネルからclientを削除
      チャンネルの参加者が0になったらチャンネル削除
      client削除
  */
  void removeclient(int fd);
  Client* getClientByFd(int fd);
  Client* getClientByNick(const std::string& nick);
  bool nickExists(const std::string& nick) const;
  void updateNick(Client& client, const std::string& newNick);
  Channel* getChannel(const std::string& name);
  Channel* getOrCreateChannel(const std::string& name);
  void removeChannel(const std::string& name);

 private:
  std::string _password;
  std::map<std::string, Channel*> _channels;
  /*
  clientの管理は委譲する
  これはfdからの検索とnickからの検索のためにmapを2つ持っているため
  serverstateから直接管理するのを避けるためである
  公開APIは窓口として、実管理はClientRegistryに任せる
  */
  ClientRegistry _client;
};

#endif
