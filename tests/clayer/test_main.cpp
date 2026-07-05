#include <iostream>
#include <string>
#include <vector>

#include "c/Channel.hpp"
#include "c/ChannelModes.hpp"
#include "c/Client.hpp"
#include "c/ServerState.hpp"
#include "c/Utils.hpp"

namespace {

int g_failed = 0;
int g_passed = 0;

void expectTrue(bool condition, const std::string& expr,
                const std::string& file, int line) {
  if (condition) {
    ++g_passed;
    return;
  }
  ++g_failed;
  std::cout << file << ":" << line << ": expected true: " << expr << std::endl;
}

void expectFalse(bool condition, const std::string& expr,
                 const std::string& file, int line) {
  expectTrue(!condition, "!(" + expr + ")", file, line);
}

template <typename T, typename U>
void expectEqual(const T& expected, const U& actual, const std::string& expr,
                 const std::string& file, int line) {
  if (expected == actual) {
    ++g_passed;
    return;
  }
  ++g_failed;
  std::cout << file << ":" << line << ": expected " << expr << std::endl;
}

#define EXPECT_TRUE(expr) expectTrue((expr), #expr, __FILE__, __LINE__)
#define EXPECT_FALSE(expr) expectFalse((expr), #expr, __FILE__, __LINE__)
#define EXPECT_EQ(expected, actual)                                     \
  expectEqual((expected), (actual), #expected " == " #actual, __FILE__, \
              __LINE__)

Client* addClientWithNick(ServerState& state, int fd, const std::string& nick) {
  state.addClient(fd, "host.example");
  Client* client = state.getClientByFd(fd);
  if (client) {
    state.updateNick(*client, nick);
  }
  return client;
}

bool hasChannel(Client* client, Channel* channel) {
  std::vector<Channel*> channels = client->getChannels();
  for (std::vector<Channel*>::const_iterator it = channels.begin();
       it != channels.end(); ++it) {
    if (*it == channel) {
      return true;
    }
  }
  return false;
}

void testClientRegistryNickUpdate() {
  ServerState state("pw");
  Client* taro = addClientWithNick(state, 10, "taro");
  Client* hanako = addClientWithNick(state, 11, "hanako");

  EXPECT_TRUE(taro != NULL);
  EXPECT_TRUE(hanako != NULL);
  EXPECT_EQ(taro, state.getClientByFd(10));
  state.addClient(10, "other.example");
  EXPECT_EQ(taro, state.getClientByFd(10));
  EXPECT_EQ(std::string("host.example"), taro->getHost());
  EXPECT_EQ(taro, state.getClientByNick("taro"));
  EXPECT_EQ(std::string("taro"), taro->getNick());
  EXPECT_FALSE(state.updateNick(*hanako, "taro"));
  EXPECT_FALSE(state.updateNick(*hanako, "TARO"));
  EXPECT_TRUE(state.updateNick(*taro, "TARO"));
  EXPECT_EQ(taro, state.getClientByNick("taro"));
  EXPECT_EQ(std::string("TARO"), taro->getNick());
}

void testClientFullPrefix() {
  Client client(20, "irc.example");

  client._unsafe_setNick("taro");
  client.setUsername("user");
  client.setRealname("Real Name");

  EXPECT_EQ(std::string("irc.example"), client.getHost());
  EXPECT_EQ(std::string("taro!user@irc.example"), client.getFullPrefix());

  client.setHost("remote.example");
  EXPECT_EQ(std::string("taro!user@remote.example"), client.getFullPrefix());
}

void testChannelModesAndLocalState() {
  Client client(20, "local.example");
  Channel channel("#local");
  ChannelModes& modes = channel.getModes();

  EXPECT_FALSE(modes.isInviteOnly());
  EXPECT_FALSE(modes.isTopicRestricted());
  EXPECT_FALSE(modes.hasKey());
  EXPECT_EQ(-1, modes.getLimit());

  modes.setInviteOnly(true);
  modes.setTopicRestricted(true);
  modes.setKey("secret");
  modes.setLimit(42);
  EXPECT_TRUE(modes.isInviteOnly());
  EXPECT_TRUE(modes.isTopicRestricted());
  EXPECT_TRUE(modes.hasKey());
  EXPECT_EQ(std::string("secret"), modes.getKey());
  EXPECT_EQ(42, modes.getLimit());

  modes.setLimit(0);
  EXPECT_EQ(42, modes.getLimit());
  modes.setLimit(-2);
  EXPECT_EQ(42, modes.getLimit());
  modes.setLimit(-1);
  EXPECT_EQ(-1, modes.getLimit());

  modes.unSetKey();
  modes.unSetLimit();
  EXPECT_FALSE(modes.hasKey());
  EXPECT_EQ(std::string(""), modes.getKey());
  EXPECT_EQ(-1, modes.getLimit());

  channel.setTopic("topic");
  EXPECT_EQ(std::string("topic"), channel.getTopic());
  channel._unsafe_addMember(&client);
  channel.setOperator(&client, true);
  EXPECT_TRUE(channel.isOperator(&client));
  channel.addInvite(&client);
  EXPECT_TRUE(channel.isInvited(&client));
  channel.removeInvite(&client);
  EXPECT_FALSE(channel.isInvited(&client));
}

void testServerStateMembership() {
  ServerState state("pw");
  Client* taro = addClientWithNick(state, 30, "taro");
  Client* hanako = addClientWithNick(state, 31, "hanako");

  Channel* channel = state.addClientToChannel(taro, "#test");
  EXPECT_TRUE(channel != NULL);
  EXPECT_EQ(channel, state.getChannel("#test"));
  EXPECT_TRUE(channel->hasMember(taro));
  EXPECT_TRUE(hasChannel(taro, channel));
  EXPECT_TRUE(channel->isOperator(taro));
  EXPECT_EQ(static_cast<size_t>(1), channel->memberCount());

  Channel* duplicateJoin = state.addClientToChannel(taro, "#test");
  EXPECT_EQ(channel, duplicateJoin);
  EXPECT_EQ(static_cast<size_t>(1), channel->memberCount());
  EXPECT_TRUE(channel->isOperator(taro));

  Channel* sameChannel = state.addClientToChannel(hanako, "#test");
  EXPECT_EQ(channel, sameChannel);
  EXPECT_TRUE(channel->hasMember(hanako));
  EXPECT_TRUE(hasChannel(hanako, channel));
  EXPECT_FALSE(channel->isOperator(hanako));

  state.removeClientFromChannel(hanako, "#test");
  EXPECT_FALSE(channel->hasMember(hanako));
  EXPECT_FALSE(hasChannel(hanako, channel));
  EXPECT_EQ(channel, state.getChannel("#test"));

  state.removeClientFromChannel(taro, "#test");
  EXPECT_TRUE(state.getChannel("#test") == NULL);
}

void testInviteCleanup() {
  ServerState state("pw");
  Client* inviter = addClientWithNick(state, 40, "inviter");
  Client* invited = addClientWithNick(state, 41, "invited");

  Channel* joined = state.addClientToChannel(inviter, "#joined");
  Channel* inviteOnly = state.getOrCreateChannel("#invite-only");
  state.inviteClientToChannel(invited, inviteOnly);
  EXPECT_TRUE(inviteOnly->isInvited(invited));

  inviteOnly->getModes().setInviteOnly(false);
  state.inviteClientToChannel(invited, inviteOnly);
  EXPECT_TRUE(inviteOnly->isInvited(invited));
  inviteOnly->getModes().setInviteOnly(true);
  EXPECT_TRUE(inviteOnly->isInvited(invited));
  inviteOnly->getModes().setInviteOnly(false);
  inviteOnly->getModes().setInviteOnly(true);
  EXPECT_TRUE(inviteOnly->isInvited(invited));

  Channel* joinedByInvite = state.addClientToChannel(invited, "#invite-only");
  EXPECT_EQ(inviteOnly, joinedByInvite);
  EXPECT_FALSE(inviteOnly->isInvited(invited));
  state.inviteClientToChannel(invited, inviteOnly);
  EXPECT_TRUE(inviteOnly->isInvited(invited));

  state.removeClient(41);
  EXPECT_FALSE(inviteOnly->isInvited(invited));
  EXPECT_TRUE(state.getClientByFd(41) == NULL);
  EXPECT_TRUE(state.getClientByNick("invited") == NULL);
  EXPECT_TRUE(joined->hasMember(inviter));
}

void testRemoveClientCleanup() {
  ServerState state("pw");
  Client* taro = addClientWithNick(state, 50, "taro");
  Client* hanako = addClientWithNick(state, 51, "hanako");

  Channel* shared = state.addClientToChannel(taro, "#shared");
  state.addClientToChannel(hanako, "#shared");
  Channel* solo = state.addClientToChannel(taro, "#solo");
  state.inviteClientToChannel(taro, shared);

  state.removeClient(50);
  EXPECT_TRUE(state.getClientByFd(50) == NULL);
  EXPECT_TRUE(state.getClientByNick("taro") == NULL);
  EXPECT_FALSE(shared->hasMember(taro));
  EXPECT_FALSE(shared->isInvited(taro));
  EXPECT_TRUE(shared->hasMember(hanako));
  EXPECT_TRUE(state.getChannel("#shared") == shared);
  EXPECT_TRUE(state.getChannel("#solo") == NULL);
  (void)solo;
}

void testNameValidationHelpers() {
  EXPECT_TRUE(isValidNickname("taro"));
  EXPECT_TRUE(isValidNickname("nick-name"));
  EXPECT_TRUE(isValidNickname("[abc]"));
  EXPECT_TRUE(isValidNickname("abcdefghi"));
  EXPECT_FALSE(isValidNickname(""));
  EXPECT_FALSE(isValidNickname("#user"));
  EXPECT_FALSE(isValidNickname("1user"));
  EXPECT_FALSE(isValidNickname("nick,name"));
  EXPECT_FALSE(isValidNickname("abcdefghij"));

  EXPECT_TRUE(isValidChannelName("#room"));
  EXPECT_TRUE(isValidChannelName("#room-name"));
  EXPECT_TRUE(isValidChannelName("#[]"));
  EXPECT_FALSE(isValidChannelName(""));
  EXPECT_FALSE(isValidChannelName("#"));
  EXPECT_FALSE(isValidChannelName("room"));
  EXPECT_FALSE(isValidChannelName("+room"));
  EXPECT_FALSE(isValidChannelName("#bad,name"));
  EXPECT_FALSE(isValidChannelName("#bad:name"));
  EXPECT_FALSE(isValidChannelName("#bad name"));
}

void runTest(const std::string& name, void (*test)()) {
  int failedBefore = g_failed;
  test();
  if (g_failed == failedBefore) {
    std::cout << "[PASS] " << name << std::endl;
  } else {
    std::cout << "[FAIL] " << name << std::endl;
  }
}

}  // namespace

int main() {
  runTest("client registry nick update", testClientRegistryNickUpdate);
  runTest("client full prefix", testClientFullPrefix);
  runTest("channel modes and local state", testChannelModesAndLocalState);
  runTest("server state membership", testServerStateMembership);
  runTest("invite cleanup", testInviteCleanup);
  runTest("remove client cleanup", testRemoveClientCleanup);
  runTest("name validation helpers", testNameValidationHelpers);

  std::cout << "Assertions passed: " << g_passed << std::endl;
  if (g_failed != 0) {
    std::cout << "Assertions failed: " << g_failed << std::endl;
    return 1;
  }
  return 0;
}
