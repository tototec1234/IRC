#ifndef SERVERSTATE_HPP
#define SERVERSTATE_HPP

#include <map>
#include <string>

#include "Channel.hpp"
#include "Client.hpp"
#include "ClientRegistry.hpp"
#include "Utils.hpp"

class ServerState {
 public:
  ServerState(const std::string& password);
  ~ServerState();

  const std::string& getPassword() const;
  void addClient(int fd, const std::string& host);
  Channel* addClientToChannel(Client* client, const std::string& channelName);
  /*
    client削除によりchannelの参加者が0になった場合はchannelも削除する
  */
  void removeClientFromChannel(Client* client, const std::string& channelName);
  void inviteClientToChannel(Client* client, Channel* channel);
  void removeInviteFromChannel(Client* client, Channel* channel);
  void removeClientFromAllInvites(Client* client);
  /*
   * @summary clientが切断、またはQUITによりserverから離脱した場合の処理
   * @param fd 切断されたclientのファイルディスクリプタ
   * @details
   * clientが所属している全てのchannelからclientを削除
   * channelのinviteリストからclientを削除
   * clientをclient registryから削除
   * channel参加者が0になったchannelは削除
   */
  void removeClient(int fd);
  Client* getClientByFd(int fd);
  Client* getClientByNick(const std::string& nick);
  bool nickExists(const std::string& nick) const;
  /*
    B層向けのnick更新Facade API。
    実際のnick辞書更新はClientRegistryに委譲する。
    B層はClientRegistryを直接触らず、この関数を使う。
  */
  bool updateNick(Client& client, const std::string& newNick);
  Channel* getChannel(const std::string& name);
  Channel* getOrCreateChannel(const std::string& name);
  void removeChannelIfEmpty(const std::string& name);

 private:
  typedef std::map<std::string, Channel*, IrcStringCompare> ChannelMap;
  typedef ChannelMap::iterator ChannelMapIt;
  std::string _password;
  ChannelMap _channels;
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
