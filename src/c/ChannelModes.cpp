#include "c/ChannelModes.hpp"

ChannelModes::ChannelModes()
    : _inviteOnly(false), _topicRestricted(false), _hasKey(false), _limit(-1) {}

ChannelModes::~ChannelModes() {}

bool ChannelModes::isInviteOnly() const { return _inviteOnly; }

void ChannelModes::setInviteOnly(bool inviteOnly) { _inviteOnly = inviteOnly; }

bool ChannelModes::isTopicRestricted() const { return _topicRestricted; }

void ChannelModes::setTopicRestricted(bool topicRestricted) {
  _topicRestricted = topicRestricted;
}

bool ChannelModes::hasKey() const { return _hasKey; }

void ChannelModes::setKey(const std::string& key) {
  _hasKey = true;
  _key = key;
}

std::string ChannelModes::getKey() const { return _key; }

int ChannelModes::getLimit() const { return _limit; }

void ChannelModes::setLimit(int limit) {
  if (limit >= 1 || limit == -1) {
    _limit = limit;
  }
}

void ChannelModes::unSetKey() {
  _hasKey = false;
  _key.clear();
}

void ChannelModes::unSetLimit() {
  _limit = -1;  // -1 indicates no limit
}
