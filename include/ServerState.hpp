#ifndef SERVERSTATE_HPP
#define SERVERSTATE_HPP

#include <map>
#include <string>

#include "Channel.hpp"
#include "Client.hpp"
#include "ClientRegistry.hpp"

class ServerState {
 public:
  ServerState(const std::string& password);
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
  /*
    clientのニックネームを更新する
    返り値がvoidだと、呼び出し側はnickExistsを呼び出して重複を確認してから
    updateNickを呼び出す必要がある
    ここではmapのInsert時の戻り値を利用して重複があった場合は
    boolを返すような設計に変更する
  */
  bool updateNick(Client& client, const std::string& newNick);
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

  ServerState();
  ServerState(const ServerState&);
  ServerState& operator=(const ServerState&);
};

#endif
