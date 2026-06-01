#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

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
  Client();
  ~Client();

  int getFd() const;
  const std::string& getNick() const;
  const std::string& getUsername() const;
  const std::string& getRealname() const;
  const std::string& getHost() const;
  std::string getFullPrefix() const;
  void setUsername(const std::string& username);
  void setRealname(const std::string& realname);
  void setHost(const std::string& host);
  /*
  ステートの管理が独立したbool値によって行われているが
  unregistered -> passok -> registered という遷移なので
  enumで管理するのが自然かもしれない
  現状のAPIが具体的にどの局面でどのように使い分けられるかがわかりづらい
  */
  void setPassOk(bool);
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
  bool _registered;
};

#endif
