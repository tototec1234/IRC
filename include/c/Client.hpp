#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <set>
#include <string>
#include <vector>

class Channel;
/*
  clientが所属channelを管理しない設計になっているが、
  serverstateを親としてみたときchannelとclientは子供同士の関係であり
  双方向に参照できても問題なさそう
  client削除時に所属channelからclientを削除する処理が必要になるが
  channel側でのみclientを管理していると全てのchannelを走査して
  clientを削除する必要があるためchannel数に比例して処理時間が増える
  そのため、client側からも所属channelを管理すべきではないか
*/

class Client {
 public:
  Client(int fd);
  ~Client();

  int getFd() const;
  const std::string& getNick() const;
  const std::string& getUsername() const;
  const std::string& getRealname() const;
  const std::string& getHost() const;
  /*
  DTOパターンでクライアントの所属チャンネルを取得するための関数。
  一意に識別される保証はクラス内で行い、vectorを返す（set→vector）
  */
  std::vector<Channel*> getChannels() const;
  std::string getFullPrefix() const;
  void setUsername(const std::string& username);
  void setRealname(const std::string& realname);
  void setHost(const std::string& host);
  /**
   * @brief 更新されたNICKをClientにキャッシュします。
   * @warning この関数は直接呼び出さないでください。
   * 必ず ServerState::updateNick()
   * を経由して、一意性の検証を行った後に呼び出すこと。
   */
  void _unsafe_setNick(const std::string& nick);
  void _unsafe_joinChannel(Channel* channel);
  void _unsafe_leaveChannel(Channel* channel);
  void setPassOk(bool passOk);
  bool isPassOk() const;
  bool isRegistered() const;
  bool canRegister() const;
  void markRegistered();

 private:
  std::string _nickname;
  std::string _username;
  std::string _realname;
  std::string _host;
  int _fd;
  bool _passOk;
  // marked when welcome message is sent.
  // this is used to prevent sending welcome message multiple times when client
  // sends multiple NICK commands.
  bool _registered;
  std::set<Channel*> _channels;

  Client();
};

#endif
