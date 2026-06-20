#include "b/DisconnectNotifier.hpp"

#include <set>
#include <string>
#include <vector>

#include "b/ReplyBuilder.hpp"
#include "c/Channel.hpp"
#include "c/Client.hpp"
#include "c/ServerState.hpp"

DisconnectNotifier::DisconnectNotifier() {}

DisconnectNotifier::~DisconnectNotifier() {}

CommandResult DisconnectNotifier::build(const DisconnectEvent& event,
                                        ServerState& state) {
  CommandResult result;
  Client* client = state.getClientByFd(event.fd);
  if (!client) {
    return result;
  }

  const std::string prefix = client->getFullPrefix();
  const std::string quitMessage = ReplyBuilder::quit(prefix, event.reason);
  std::set<int> notifiedFds;
  std::vector<Channel*> channels = client->getChannels();

  for (std::vector<Channel*>::const_iterator channelIt = channels.begin();
       channelIt != channels.end(); ++channelIt) {
    Channel* channel = *channelIt;
    if (!channel) {
      continue;
    }
    std::vector<Client*> members = channel->getMembers();
    for (std::vector<Client*>::const_iterator memberIt = members.begin();
         memberIt != members.end(); ++memberIt) {
      Client* member = *memberIt;
      if (!member || member->getFd() == event.fd) {
        continue;
      }
      if (notifiedFds.insert(member->getFd()).second) {
        result.addReply(member->getFd(), quitMessage);
      }
    }
  }

  return result;
}
