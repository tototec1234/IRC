#ifndef CHANNELMODES_HPP
#define CHANNELMODES_HPP

#include <string>

class ChannelModes {
 public:
  ChannelModes();
  ~ChannelModes();

  bool isInviteOnly() const;
  void setInviteOnly(bool inviteOnly);
  bool isTopicRestricted() const;
  void setTopicRestricted(bool topicRestricted);
  bool hasKey() const;
  void setKey(const std::string& key);
  std::string getKey() const;
  int getLimit() const;
  // 最大値どうする？
  // 呼び出し側が適切な値を渡すこと
  void setLimit(int limit);
  void unsetKey();
  void unsetLimit();

 private:
  bool _inviteOnly;
  bool _topicrestricted;
  bool _haskey;
  std::string _key;
  int _limit;  // -1 indicates no limit
};

#endif
