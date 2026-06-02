#include "ChannelModes.hpp"

ChannelModes::ChannelModes()
    : _inviteOnly(false), _topicrestricted(false), _haskey(false), _limit(-1) {}

ChannelModes::~ChannelModes() {}

bool ChannelModes::isInviteOnly() const { return _inviteOnly; }

void ChannelModes::setInviteOnly(bool inviteOnly) { _inviteOnly = inviteOnly; }

bool ChannelModes::isTopicRestricted() const { return _topicrestricted; }

void ChannelModes::setTopicRestricted(bool topicRestricted) {
  _topicrestricted = topicRestricted;
}

bool ChannelModes::hasKey() const { return _haskey; }

void ChannelModes::setKey(const std::string& key) {
  _haskey = true;
  _key = key;
}

std::string ChannelModes::getKey() const { return _key; }

int ChannelModes::getLimit() const { return _limit; }

void ChannelModes::setLimit(int limit) { _limit = limit; }

void ChannelModes::unsetKey() {
  _haskey = false;
  _key.clear();
}

void ChannelModes::unsetLimit() {
  _limit = -1;  // -1 indicates no limit
}
