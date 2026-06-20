#include <iostream>
#include <string>
#include <vector>

#include "c/Client.hpp"
#include "b/CommandDispatcher.hpp"
#include "b/CommandResult.hpp"
#include "b/Message.hpp"
#include "b/Parser.hpp"
#include "b/ReplyBuilder.hpp"
#include "c/ServerState.hpp"

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

Message makeMessage(const std::string& command, const std::string& p0) {
  std::vector<std::string> params;
  params.push_back(p0);
  return Message(command, params);
}

Message makeMessage(const std::string& command, const std::string& p0,
                    const std::string& p1) {
  std::vector<std::string> params;
  params.push_back(p0);
  params.push_back(p1);
  return Message(command, params);
}

Message makeUserMessage() {
  std::vector<std::string> params;
  params.push_back("user");
  params.push_back("0");
  params.push_back("*");
  params.push_back("Real Name");
  return Message("USER", params);
}

struct TestClient {
  TestClient(int clientFd, const std::string& clientNick)
      : fd(clientFd), nick(clientNick) {}

  int fd;
  std::string nick;
};

struct TestContext {
  TestContext() : state("pw") {}

  TestClient addClient(int fd) {
	state.addClient(fd, "client.example");
	return TestClient(fd, "");
  }

  TestClient registerClient(int fd, const std::string& nick) {
	addClient(fd);
	dispatch(fd, makeMessage("PASS", "pw"));
	dispatch(fd, makeMessage("NICK", nick));
	dispatch(fd, makeUserMessage());
	return TestClient(fd, nick);
  }

  CommandResult dispatch(int fd, const Message& msg) {
	return dispatcher.dispatch(fd, msg, state);
  }

  Client* client(int fd) { return state.getClientByFd(fd); }

  CommandResult join(int fd, const std::string& channelName) {
	return dispatch(fd, makeMessage("JOIN", channelName));
  }

  ServerState state;
  CommandDispatcher dispatcher;
};

void expectContains(const std::string& text, const std::string& needle,
					const std::string& file, int line) {
  expectTrue(text.find(needle) != std::string::npos, "contains " + needle, file,
			 line);
}

#define EXPECT_CONTAINS(text, needle) \
  expectContains((text), (needle), __FILE__, __LINE__)

void testParserBasicMessage() {
  Message msg = Parser::parse("PRIVMSG #room :hello world\r\n");

  EXPECT_EQ(std::string("PRIVMSG"), msg.getCommand());
  EXPECT_EQ(static_cast<size_t>(2), msg.getParamCount());
  EXPECT_EQ(std::string("#room"), msg.getSingleParam(0));
  EXPECT_EQ(std::string("hello world"), msg.getSingleParam(1));
}

void testPingReturnsPong() {
  TestContext ctx;
  TestClient taro = ctx.addClient(10);

  CommandResult result = ctx.dispatch(taro.fd, makeMessage("PING", "token"));

  EXPECT_FALSE(result.shouldDisconnect);
  EXPECT_EQ(static_cast<size_t>(1), result.replies.size());
  EXPECT_EQ(taro.fd, result.replies[0].fd);
  EXPECT_CONTAINS(result.replies[0].message, " PONG ");
  EXPECT_CONTAINS(result.replies[0].message, "token");
}

void testRegistrationFlowUsesRealCState() {
  TestContext ctx;
  TestClient taro = ctx.addClient(20);
  Client* client = ctx.client(taro.fd);

  EXPECT_TRUE(client != NULL);
  CommandResult passResult = ctx.dispatch(taro.fd, makeMessage("PASS", "pw"));
  CommandResult nickResult =
	  ctx.dispatch(taro.fd, makeMessage("NICK", "taro"));
  CommandResult userResult = ctx.dispatch(taro.fd, makeUserMessage());

  EXPECT_EQ(static_cast<size_t>(0), passResult.replies.size());
  EXPECT_EQ(static_cast<size_t>(0), nickResult.replies.size());
  EXPECT_TRUE(client->isPassOk());
  EXPECT_EQ(std::string("taro"), client->getNick());
  EXPECT_EQ(std::string("taro!user@client.example"), client->getFullPrefix());
  EXPECT_EQ(client, ctx.state.getClientByNick("taro"));
  EXPECT_TRUE(client->isRegistered());
  EXPECT_EQ(static_cast<size_t>(1), userResult.replies.size());
  EXPECT_CONTAINS(userResult.replies[0].message, " 001 taro ");
}

void testNickBeforePassIsRejected() {
  TestContext ctx;
  TestClient taro = ctx.addClient(25);
  Client* client = ctx.client(taro.fd);

  CommandResult result = ctx.dispatch(taro.fd, makeMessage("NICK", "taro"));

  EXPECT_TRUE(client != NULL);
  EXPECT_FALSE(client->isPassOk());
  EXPECT_TRUE(client->getNick().empty());
  EXPECT_TRUE(ctx.state.getClientByNick("taro") == NULL);
  EXPECT_FALSE(client->isRegistered());
  EXPECT_EQ(static_cast<size_t>(1), result.replies.size());
  EXPECT_CONTAINS(result.replies[0].message, " 464 ");
}

void testUserBeforePassIsRejected() {
  TestContext ctx;
  TestClient taro = ctx.addClient(26);
  Client* client = ctx.client(taro.fd);

  CommandResult result = ctx.dispatch(taro.fd, makeUserMessage());

  EXPECT_TRUE(client != NULL);
  EXPECT_FALSE(client->isPassOk());
  EXPECT_TRUE(client->getUsername().empty());
  EXPECT_TRUE(client->getRealname().empty());
  EXPECT_FALSE(client->isRegistered());
  EXPECT_EQ(static_cast<size_t>(1), result.replies.size());
  EXPECT_CONTAINS(result.replies[0].message, " 464 ");
}

void testNickConflictReturnsNumeric() {
  TestContext ctx;
  TestClient taro = ctx.addClient(30);
  TestClient hanako = ctx.addClient(31);

  ctx.dispatch(taro.fd, makeMessage("PASS", "pw"));
  ctx.dispatch(hanako.fd, makeMessage("PASS", "pw"));
  ctx.dispatch(taro.fd, makeMessage("NICK", "taro"));
  CommandResult result = ctx.dispatch(hanako.fd, makeMessage("NICK", "TARO"));

  EXPECT_EQ(static_cast<size_t>(1), result.replies.size());
  EXPECT_CONTAINS(result.replies[0].message, " 433 ");
  EXPECT_TRUE(ctx.client(hanako.fd)->getNick().empty());
}

void testNotRegisteredReplyFormat() {
  TestContext ctx;
  TestClient taro = ctx.addClient(40);
  Client* client = ctx.client(taro.fd);

	std::string reply = ReplyBuilder::noRegistered(*client);

	EXPECT_TRUE(client != NULL);
	EXPECT_FALSE(client->isRegistered());
	EXPECT_CONTAINS(reply, " 451 ");
	EXPECT_CONTAINS(reply, " 451 * ");
	EXPECT_CONTAINS(reply, "You have not registered");
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

void testJoinBeforeRegistrationReturns451() {
	TestContext ctx;
	TestClient taro = ctx.addClient(42);
	Client* client = ctx.client(taro.fd);
  
	CommandResult result = ctx.join(taro.fd, "#room42Tokyo");
  
	EXPECT_FALSE(client->isRegistered());
	EXPECT_EQ(static_cast<size_t>(1), result.replies.size());
	EXPECT_EQ(taro.fd, result.replies[0].fd);
	EXPECT_CONTAINS(result.replies[0].message, " 451 ");
  }


  void testJoinAfterRegistrationEchoesToSelf() {
	TestContext ctx;
	TestClient taro = ctx.registerClient(43, "taro");
	CommandResult result = ctx.join(taro.fd, "#taros_room");
	EXPECT_TRUE(ctx.client(taro.fd)->isRegistered());
	EXPECT_TRUE(result.replies.size() >= static_cast<size_t>(1));
	EXPECT_EQ(taro.fd, result.replies[0].fd);
	EXPECT_CONTAINS(result.replies[0].message, "JOIN" );
	EXPECT_CONTAINS(result.replies[0].message, ":taro!user@client.example JOIN #taros_room" );
  }

  void testPartSingleMemberEchoesToSelf() {
	TestContext ctx;
	TestClient taro = ctx.registerClient(45, "taro");
	ctx.join(taro.fd, "#solo");

	CommandResult result = ctx.dispatch(taro.fd, makeMessage("PART", "#solo"));

	EXPECT_EQ(static_cast<size_t>(1), result.replies.size());
	EXPECT_EQ(taro.fd, result.replies[0].fd);
	EXPECT_CONTAINS(result.replies[0].message,
					":taro!user@client.example PART #solo");
	EXPECT_TRUE(ctx.state.getChannel("#solo") == NULL);
  }

  void testQuitBeforeRegistrationDisconnects() {
	TestContext ctx;
	TestClient taro = ctx.addClient(44);

	CommandResult result = ctx.dispatch(taro.fd, makeMessage("QUIT", "bye"));

  EXPECT_TRUE(result.shouldDisconnect);
  EXPECT_EQ(static_cast<size_t>(0), result.replies.size());
  }

  void testPrivmsgToNickSendsOnlyTarget() {
	TestContext ctx;
	TestClient taro = ctx.registerClient(50, "taro");
	TestClient hanako = ctx.registerClient(51, "hanako");
	ctx.registerClient(52, "jiro");

	CommandResult result =
		ctx.dispatch(taro.fd, makeMessage("PRIVMSG", hanako.nick, "hello"));

	EXPECT_EQ(static_cast<size_t>(1), result.replies.size());
	EXPECT_EQ(hanako.fd, result.replies[0].fd);
	EXPECT_CONTAINS(result.replies[0].message,
					":taro!user@client.example PRIVMSG hanako :hello");
  }

  void testPrivmsgToChannelSkipsSender() {
	TestContext ctx;
	TestClient taro = ctx.registerClient(53, "taro");
	TestClient hanako = ctx.registerClient(54, "hanako");
	ctx.join(taro.fd, "#room");
	ctx.join(hanako.fd, "#room");

	CommandResult result =
		ctx.dispatch(taro.fd, makeMessage("PRIVMSG", "#room", "hello"));

	EXPECT_EQ(static_cast<size_t>(1), result.replies.size());
	EXPECT_EQ(hanako.fd, result.replies[0].fd);
	EXPECT_CONTAINS(result.replies[0].message,
					":taro!user@client.example PRIVMSG #room :hello");
  }

  void testPrivmsgInvalidTargetReturnsNumeric() {
	TestContext ctx;
	TestClient taro = ctx.registerClient(55, "taro");

	CommandResult result =
		ctx.dispatch(taro.fd, makeMessage("PRIVMSG", "missing", "hello"));

	EXPECT_EQ(static_cast<size_t>(1), result.replies.size());
	EXPECT_EQ(taro.fd, result.replies[0].fd);
	EXPECT_CONTAINS(result.replies[0].message, " 401 ");
  }

  void testNoticeInvalidTargetReturnsNoReply() {
	TestContext ctx;
	TestClient taro = ctx.registerClient(56, "taro");

	CommandResult result =
		ctx.dispatch(taro.fd, makeMessage("NOTICE", "missing", "hello"));

	EXPECT_EQ(static_cast<size_t>(0), result.replies.size());
  }

  void testNoticeDeliversToNickAndChannel() {
	TestContext ctx;
	TestClient taro = ctx.registerClient(57, "taro");
	TestClient hanako = ctx.registerClient(58, "hanako");
	ctx.join(taro.fd, "#notice");
	ctx.join(hanako.fd, "#notice");

	CommandResult nickResult =
		ctx.dispatch(taro.fd, makeMessage("NOTICE", hanako.nick, "direct"));
	CommandResult channelResult =
		ctx.dispatch(taro.fd, makeMessage("NOTICE", "#notice", "channel"));

	EXPECT_EQ(static_cast<size_t>(1), nickResult.replies.size());
	EXPECT_EQ(hanako.fd, nickResult.replies[0].fd);
	EXPECT_CONTAINS(nickResult.replies[0].message,
					":taro!user@client.example NOTICE hanako :direct");
	EXPECT_EQ(static_cast<size_t>(1), channelResult.replies.size());
	EXPECT_EQ(hanako.fd, channelResult.replies[0].fd);
	EXPECT_CONTAINS(channelResult.replies[0].message,
					":taro!user@client.example NOTICE #notice :channel");
  }

}  // namespace

int main() {
  runTest("parser basic message", testParserBasicMessage);
  runTest("ping returns pong", testPingReturnsPong);
  runTest("registration flow uses real C state",
		  testRegistrationFlowUsesRealCState);
  runTest("nick before pass is rejected", testNickBeforePassIsRejected);
  runTest("user before pass is rejected", testUserBeforePassIsRejected);
  runTest("nick conflict returns numeric", testNickConflictReturnsNumeric);

  runTest("not registered reply format", testNotRegisteredReplyFormat);

  runTest("join before registration returns 451",
		  testJoinBeforeRegistrationReturns451);

  runTest("join after registration echoes to self",
			testJoinAfterRegistrationEchoesToSelf);
  runTest("part single member echoes to self",
			testPartSingleMemberEchoesToSelf);
  runTest("quit before registration disconnects",
			testQuitBeforeRegistrationDisconnects);
  runTest("privmsg to nick sends only target",
			testPrivmsgToNickSendsOnlyTarget);
  runTest("privmsg to channel skips sender",
			testPrivmsgToChannelSkipsSender);
  runTest("privmsg invalid target returns numeric",
			testPrivmsgInvalidTargetReturnsNumeric);
  runTest("notice invalid target returns no reply",
			testNoticeInvalidTargetReturnsNoReply);
  runTest("notice delivers to nick and channel",
			testNoticeDeliversToNickAndChannel);

  std::cout << "Assertions passed: " << g_passed << std::endl;
  if (g_failed != 0) {
	std::cout << "Assertions failed: " << g_failed << std::endl;
	return 1;
  }
  return 0;
}
