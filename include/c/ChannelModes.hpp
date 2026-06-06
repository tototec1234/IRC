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
  // Accepts 1 or greater, or -1 for no limit.
  void setLimit(int limit);
  void unSetKey();
  void unSetLimit();

 private:
  bool _inviteOnly;
  bool _topicRestricted;
  bool _hasKey;
  std::string _key;
  int _limit;  // -1 indicates no limit
};

#endif
