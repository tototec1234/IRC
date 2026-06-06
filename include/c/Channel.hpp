#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <map>
#include <set>
#include <string>
#include <vector>

#include "ChannelModes.hpp"
class Client;

struct MemberInfo {
  bool isOperator;
};

class Channel {
 public:
  Channel(const std::string& name);
  ~Channel();

  const std::string& getName() const;
  const std::string& getTopic() const;
  void setTopic(const std::string& topic);
  bool hasMember(Client* client) const;
  /**
   * @brief clientをチャンネルに追加します。
   * @warning この関数は直接呼び出さないでください。
   * 必ず ServerState::addClientToChannel()
   * を経由して、clientの所属channelと同期した状態を保つこと。
   */
  void _unsafe_addMember(Client* client);
  /**
   * @brief clientをチャンネルから削除します。
   * @warning この関数は直接呼び出さないでください。
   * 必ず ServerState::removeClientFromChannel()
   * を経由して、clientの所属channelと同期した状態を保つこと。
   */
  void _unsafe_removeMember(Client* client);
  /**
   * @brief Channel内部状態からclientを削除します。
   * @warning この関数は直接呼び出さないでください。
   * Clientの所属channelとは同期されないため、必ずServerState経由で使うこと。
   */
  void _unsafe_removeClientState(Client* client);
  /*
    メンバはmapで管理されているが、clientのリストを返すAPIにおいて
    memberinfoが露出することはカプセル化の観点から望ましくないため
    vectorに詰め替えて返すAPIとする（DTOパターン）
  */
  std::vector<Client*> getMembers() const;
  size_t memberCount() const;
  bool isOperator(Client* client) const;
  void setOperator(Client* client, bool isOperator);
  void addInvite(Client* client);
  bool isInvited(Client* client) const;
  void removeInvite(Client* client);
  ChannelModes& getModes();
  const ChannelModes& getModes() const;
  bool isEmpty() const;

 private:
  std::string _name;
  std::string _topic;
  std::map<Client*, MemberInfo> _members;

  std::set<Client*> _invitedClients;
  ChannelModes _modes;

  Channel();
  Channel(const Channel&);
  Channel& operator=(const Channel&);
};

#endif
