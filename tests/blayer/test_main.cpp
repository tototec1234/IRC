#include <iostream>
#include <string>
#include <vector>

#include "c/Client.hpp"
#include "b/CommandDispatcher.hpp"
#include "b/CommandResult.hpp"
#include "b/DisconnectEvent.hpp"
#include "b/DisconnectNotifier.hpp"
#include "b/Message.hpp"
#include "b/Parser.hpp"
#include "b/ReplyBuilder.hpp"
#include "c/ServerState.hpp"
#include "lifecycle/ConnectionHealthMonitor.hpp"

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

Message makeMessage(const std::string& command, const std::string& p0,
                    const std::string& p1, const std::string& p2) {
  std::vector<std::string> params;
  params.push_back(p0);
  params.push_back(p1);
  params.push_back(p2);
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

Message makeMessageNoParams(const std::string& command) {
  std::vector<std::string> params;
  return Message(command, params);
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

  CommandResult dispatchWithHealth(int fd, const Message& msg,
                                   ConnectionHealthMonitor& monitor) {
	return dispatcher.dispatch(fd, msg, state, monitor);
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

bool resultHasReplyToContaining(const CommandResult& result, int fd,
                                const std::string& needle) {
  for (std::vector<OutgoingMessage>::const_iterator it = result.replies.begin();
       it != result.replies.end(); ++it) {
    if (it->fd == fd && it->message.find(needle) != std::string::npos) {
      return true;
    }
  }
  return false;
}

#define EXPECT_REPLY_TO_CONTAINS(result, fd, needle) \
  EXPECT_TRUE(resultHasReplyToContaining((result), (fd), (needle)))

void testParserBasicMessage() {
  Message msg = Parser::parse("PRIVMSG #room :hello world\r\n");

  EXPECT_EQ(std::string(""), msg.getPrefix());
  EXPECT_EQ(std::string("PRIVMSG"), msg.getCommand());
  EXPECT_EQ(static_cast<size_t>(2), msg.getParamCount());
  EXPECT_EQ(std::string("#room"), msg.getSingleParam(0));
  EXPECT_EQ(std::string("hello world"), msg.getSingleParam(1));
}

void testParserFullPrefix() {
  Message msg =
      Parser::parse(":alice!user@host PRIVMSG #room :hello world\r\n");

  EXPECT_EQ(std::string("alice!user@host"), msg.getPrefix());
  EXPECT_EQ(std::string("PRIVMSG"), msg.getCommand());
  EXPECT_EQ(static_cast<size_t>(2), msg.getParamCount());
  EXPECT_EQ(std::string("#room"), msg.getSingleParam(0));
  EXPECT_EQ(std::string("hello world"), msg.getSingleParam(1));
}

void testParserServerNumericPrefix() {
  Message msg = Parser::parse(":irc.local 001 bonusbot :Welcome\r\n");

  EXPECT_EQ(std::string("irc.local"), msg.getPrefix());
  EXPECT_EQ(std::string("001"), msg.getCommand());
  EXPECT_EQ(static_cast<size_t>(2), msg.getParamCount());
  EXPECT_EQ(std::string("bonusbot"), msg.getSingleParam(0));
  EXPECT_EQ(std::string("Welcome"), msg.getSingleParam(1));
}

void testParserPrefixOnlyIsEmptyMessage() {
  Message msg = Parser::parse(":alice!user@host\r\n");

  EXPECT_EQ(std::string(""), msg.getPrefix());
  EXPECT_EQ(std::string(""), msg.getCommand());
  EXPECT_EQ(static_cast<size_t>(0), msg.getParamCount());
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

void testDispatchUnknownFdReturnsEmptyResult() {
  TestContext ctx;

  CommandResult result = ctx.dispatch(999, makeMessage("PING", "token"));

  EXPECT_FALSE(result.shouldDisconnect);
  EXPECT_EQ(static_cast<size_t>(0), result.replies.size());
}

void testPingWithoutParamUsesStarTargetBeforeNick() {
  TestContext ctx;
  TestClient taro = ctx.addClient(11);

  CommandResult result = ctx.dispatch(taro.fd, makeMessageNoParams("PING"));

  EXPECT_FALSE(result.shouldDisconnect);
  EXPECT_EQ(static_cast<size_t>(1), result.replies.size());
  EXPECT_EQ(taro.fd, result.replies[0].fd);
  EXPECT_CONTAINS(result.replies[0].message, " 461 * PING ");
}

void testUnknownCommandUsesStarTargetBeforeNick() {
  TestContext ctx;
  TestClient taro = ctx.addClient(12);

  CommandResult result = ctx.dispatch(taro.fd, makeMessageNoParams("WAT"));

  EXPECT_FALSE(result.shouldDisconnect);
  EXPECT_EQ(static_cast<size_t>(1), result.replies.size());
  EXPECT_EQ(taro.fd, result.replies[0].fd);
  EXPECT_CONTAINS(result.replies[0].message, " 421 * WAT ");
}

void testUnknownCommandUsesNickTargetAfterNick() {
  TestContext ctx;
  TestClient taro = ctx.addClient(13);
  ctx.dispatch(taro.fd, makeMessage("PASS", "pw"));
  ctx.dispatch(taro.fd, makeMessage("NICK", "taro"));

  CommandResult result = ctx.dispatch(taro.fd, makeMessageNoParams("WAT"));

  EXPECT_FALSE(result.shouldDisconnect);
  EXPECT_EQ(static_cast<size_t>(1), result.replies.size());
  EXPECT_EQ(taro.fd, result.replies[0].fd);
  EXPECT_CONTAINS(result.replies[0].message, " 421 taro WAT ");
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

void testInvalidNicknamesReturn432WithoutUpdatingState() {
  TestContext ctx;
  TestClient taro = ctx.addClient(32);
  ctx.dispatch(taro.fd, makeMessage("PASS", "pw"));

  const char* invalidNicks[] = {
      "#user", "1user", "", "bad,name", "abcdefghij"};
  for (size_t i = 0; i < sizeof(invalidNicks) / sizeof(invalidNicks[0]);
       ++i) {
    CommandResult result =
        ctx.dispatch(taro.fd, makeMessage("NICK", invalidNicks[i]));

    EXPECT_EQ(static_cast<size_t>(1), result.replies.size());
    EXPECT_CONTAINS(result.replies[0].message, " 432 ");
    EXPECT_TRUE(ctx.client(taro.fd)->getNick().empty());
    EXPECT_TRUE(ctx.state.getClientByNick(invalidNicks[i]) == NULL);
  }
}

void testValidNicknameBoundariesAreAccepted() {
  TestContext ctx;
  TestClient first = ctx.addClient(33);
  TestClient second = ctx.addClient(34);
  TestClient third = ctx.addClient(35);
  ctx.dispatch(first.fd, makeMessage("PASS", "pw"));
  ctx.dispatch(second.fd, makeMessage("PASS", "pw"));
  ctx.dispatch(third.fd, makeMessage("PASS", "pw"));

  CommandResult firstResult =
      ctx.dispatch(first.fd, makeMessage("NICK", "abcdefghi"));
  CommandResult secondResult =
      ctx.dispatch(second.fd, makeMessage("NICK", "[abc]"));
  CommandResult thirdResult =
      ctx.dispatch(third.fd, makeMessage("NICK", "nick-name"));

  EXPECT_EQ(static_cast<size_t>(0), firstResult.replies.size());
  EXPECT_EQ(static_cast<size_t>(0), secondResult.replies.size());
  EXPECT_EQ(static_cast<size_t>(0), thirdResult.replies.size());
  EXPECT_TRUE(ctx.state.getClientByNick("abcdefghi") == ctx.client(first.fd));
  EXPECT_TRUE(ctx.state.getClientByNick("[abc]") == ctx.client(second.fd));
  EXPECT_TRUE(ctx.state.getClientByNick("nick-name") == ctx.client(third.fd));
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
	EXPECT_CONTAINS(result.replies[0].message, " 451 * ");
  }

  void testJoinBeforeRegistrationUsesNickTargetAfterNick() {
	TestContext ctx;
	TestClient taro = ctx.addClient(46);
	ctx.dispatch(taro.fd, makeMessage("PASS", "pw"));
	ctx.dispatch(taro.fd, makeMessage("NICK", "taro"));
	Client* client = ctx.client(taro.fd);

	CommandResult result = ctx.join(taro.fd, "#room46");

	EXPECT_TRUE(client != NULL);
	EXPECT_FALSE(client->isRegistered());
	EXPECT_EQ(static_cast<size_t>(1), result.replies.size());
	EXPECT_EQ(taro.fd, result.replies[0].fd);
	EXPECT_CONTAINS(result.replies[0].message, " 451 taro ");
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

  void testJoinSuccessSendsTopicNamesAndNo999() {
	TestContext ctx;
	TestClient taro = ctx.registerClient(47, "taro");

	CommandResult result = ctx.join(taro.fd, "#names");

	EXPECT_TRUE(result.replies.size() >= static_cast<size_t>(4));
	EXPECT_REPLY_TO_CONTAINS(result, taro.fd,
							 ":taro!user@client.example JOIN #names");
	EXPECT_REPLY_TO_CONTAINS(result, taro.fd,
							 " 331 taro #names :No topic is set");
	EXPECT_REPLY_TO_CONTAINS(result, taro.fd,
							 " 353 taro = #names :@taro");
	EXPECT_REPLY_TO_CONTAINS(result, taro.fd,
							 " 366 taro #names :End of /NAMES list");
	for (std::vector<OutgoingMessage>::const_iterator it = result.replies.begin();
		 it != result.replies.end(); ++it) {
	  EXPECT_FALSE(it->message.find(" 999 ") != std::string::npos);
	}
  }

  void testJoinNamesMarksOperators() {
	TestContext ctx;
	TestClient taro = ctx.registerClient(48, "taro");
	TestClient hanako = ctx.registerClient(49, "hanako");
	ctx.join(taro.fd, "#namesop");

	CommandResult result = ctx.join(hanako.fd, "#namesop");

	EXPECT_REPLY_TO_CONTAINS(result, hanako.fd,
							 " 353 hanako = #namesop ");
	EXPECT_REPLY_TO_CONTAINS(result, hanako.fd, "@taro");
	EXPECT_REPLY_TO_CONTAINS(result, hanako.fd, "hanako");
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

  void testInviteDelegatesAndPreservesBehavior() {
	TestContext ctx;
	TestClient taro = ctx.registerClient(70, "taro");
	TestClient hanako = ctx.registerClient(71, "hanako");
	ctx.join(taro.fd, "#invite");

	CommandResult result =
		ctx.dispatch(taro.fd, makeMessage("INVITE", hanako.nick, "#invite"));

	EXPECT_EQ(static_cast<size_t>(2), result.replies.size());
	EXPECT_REPLY_TO_CONTAINS(result, taro.fd, " 341 taro hanako #invite");
	EXPECT_REPLY_TO_CONTAINS(result, hanako.fd,
							 ":taro!user@client.example INVITE hanako :#invite");
	EXPECT_TRUE(ctx.state.getChannel("#invite")->isInvited(ctx.client(hanako.fd)));
  }

  void testTopicDelegatesAndPreservesBehavior() {
	TestContext ctx;
	TestClient taro = ctx.registerClient(72, "taro");
	TestClient hanako = ctx.registerClient(73, "hanako");
	ctx.join(taro.fd, "#topic");
	ctx.join(hanako.fd, "#topic");

	CommandResult result =
		ctx.dispatch(taro.fd, makeMessage("TOPIC", "#topic", "new topic"));

	EXPECT_EQ(std::string("new topic"),
			  ctx.state.getChannel("#topic")->getTopic());
	EXPECT_EQ(static_cast<size_t>(2), result.replies.size());
	EXPECT_REPLY_TO_CONTAINS(result, taro.fd,
							 ":taro!user@client.example TOPIC #topic :new topic");
	EXPECT_REPLY_TO_CONTAINS(result, hanako.fd,
							 ":taro!user@client.example TOPIC #topic :new topic");
  }

  void testKickSuccessBroadcastsAndRemovesTarget() {
	TestContext ctx;
	TestClient taro = ctx.registerClient(74, "taro");
	TestClient hanako = ctx.registerClient(75, "hanako");
	ctx.join(taro.fd, "#kick");
	ctx.join(hanako.fd, "#kick");

	CommandResult result =
		ctx.dispatch(taro.fd, makeMessage("KICK", "#kick", hanako.nick, "bye"));

	EXPECT_EQ(static_cast<size_t>(2), result.replies.size());
	EXPECT_REPLY_TO_CONTAINS(result, taro.fd,
							 ":taro!user@client.example KICK #kick hanako :bye");
	EXPECT_REPLY_TO_CONTAINS(result, hanako.fd,
							 ":taro!user@client.example KICK #kick hanako :bye");
	EXPECT_FALSE(ctx.state.getChannel("#kick")->hasMember(ctx.client(hanako.fd)));
  }

  void testKickNonOperatorReturns482() {
	TestContext ctx;
	TestClient taro = ctx.registerClient(76, "taro");
	TestClient hanako = ctx.registerClient(77, "hanako");
	TestClient jiro = ctx.registerClient(78, "jiro");
	ctx.join(taro.fd, "#kickerr");
	ctx.join(hanako.fd, "#kickerr");
	ctx.join(jiro.fd, "#kickerr");

	CommandResult result =
		ctx.dispatch(hanako.fd, makeMessage("KICK", "#kickerr", jiro.nick));

	EXPECT_EQ(static_cast<size_t>(1), result.replies.size());
	EXPECT_EQ(hanako.fd, result.replies[0].fd);
	EXPECT_CONTAINS(result.replies[0].message, " 482 ");
	EXPECT_TRUE(ctx.state.getChannel("#kickerr")->hasMember(ctx.client(jiro.fd)));
  }

  void testKickMissingTargetReturns461() {
	TestContext ctx;
	TestClient taro = ctx.registerClient(79, "taro");
	ctx.join(taro.fd, "#kickmissing");

	CommandResult result = ctx.dispatch(taro.fd, makeMessage("KICK", "#kickmissing"));

	EXPECT_EQ(static_cast<size_t>(1), result.replies.size());
	EXPECT_CONTAINS(result.replies[0].message, " 461 ");
	EXPECT_CONTAINS(result.replies[0].message, "KICK");
  }

  void testKickTargetNotInChannelReturns441() {
	TestContext ctx;
	TestClient taro = ctx.registerClient(80, "taro");
	TestClient hanako = ctx.registerClient(81, "hanako");
	ctx.join(taro.fd, "#kicknotin");

	CommandResult result =
		ctx.dispatch(taro.fd, makeMessage("KICK", "#kicknotin", hanako.nick));

	EXPECT_EQ(static_cast<size_t>(1), result.replies.size());
	EXPECT_CONTAINS(result.replies[0].message, " 441 ");
	EXPECT_TRUE(ctx.state.getChannel("#kicknotin")->hasMember(ctx.client(taro.fd)));
  }

  void testModeFlagsKeyAndLimitMutateState() {
	TestContext ctx;
	TestClient taro = ctx.registerClient(82, "taro");
	ctx.join(taro.fd, "#mode");
	Channel* channel = ctx.state.getChannel("#mode");

	ctx.dispatch(taro.fd, makeMessage("MODE", "#mode", "+i"));
	EXPECT_TRUE(channel->getModes().isInviteOnly());
	ctx.dispatch(taro.fd, makeMessage("MODE", "#mode", "-i"));
	EXPECT_FALSE(channel->getModes().isInviteOnly());
	ctx.dispatch(taro.fd, makeMessage("MODE", "#mode", "+t"));
	EXPECT_TRUE(channel->getModes().isTopicRestricted());
	ctx.dispatch(taro.fd, makeMessage("MODE", "#mode", "-t"));
	EXPECT_FALSE(channel->getModes().isTopicRestricted());
	ctx.dispatch(taro.fd, makeMessage("MODE", "#mode", "+k", "secret"));
	EXPECT_TRUE(channel->getModes().hasKey());
	EXPECT_EQ(std::string("secret"), channel->getModes().getKey());
	ctx.dispatch(taro.fd, makeMessage("MODE", "#mode", "-k"));
	EXPECT_FALSE(channel->getModes().hasKey());
	ctx.dispatch(taro.fd, makeMessage("MODE", "#mode", "+l", "10"));
	EXPECT_EQ(10, channel->getModes().getLimit());
	ctx.dispatch(taro.fd, makeMessage("MODE", "#mode", "-l"));
	EXPECT_EQ(-1, channel->getModes().getLimit());
  }

  void testModeOperatorMutatesState() {
	TestContext ctx;
	TestClient taro = ctx.registerClient(83, "taro");
	TestClient hanako = ctx.registerClient(84, "hanako");
	ctx.join(taro.fd, "#modeop");
	ctx.join(hanako.fd, "#modeop");
	Channel* channel = ctx.state.getChannel("#modeop");

	ctx.dispatch(taro.fd, makeMessage("MODE", "#modeop", "+o", hanako.nick));
	EXPECT_TRUE(channel->isOperator(ctx.client(hanako.fd)));
	ctx.dispatch(taro.fd, makeMessage("MODE", "#modeop", "-o", hanako.nick));
	EXPECT_FALSE(channel->isOperator(ctx.client(hanako.fd)));
  }

  void testModeCompoundTokensReturnErrorWithoutMutation() {
	TestContext ctx;
	TestClient taro = ctx.registerClient(85, "taro");
	ctx.join(taro.fd, "#modecompound");
	Channel* channel = ctx.state.getChannel("#modecompound");

	CommandResult first =
		ctx.dispatch(taro.fd, makeMessage("MODE", "#modecompound", "+it"));
	CommandResult second =
		ctx.dispatch(taro.fd, makeMessage("MODE", "#modecompound", "+kl",
										  "secret"));
	CommandResult third =
		ctx.dispatch(taro.fd, makeMessage("MODE", "#modecompound", "+ooo"));

	EXPECT_CONTAINS(first.replies[0].message, " 472 ");
	EXPECT_CONTAINS(second.replies[0].message, " 472 ");
	EXPECT_CONTAINS(third.replies[0].message, " 472 ");
	EXPECT_FALSE(channel->getModes().isInviteOnly());
	EXPECT_FALSE(channel->getModes().isTopicRestricted());
	EXPECT_FALSE(channel->getModes().hasKey());
  }

  void testModeMissingArgumentsReturn461() {
	TestContext ctx;
	TestClient taro = ctx.registerClient(86, "taro");
	ctx.join(taro.fd, "#modemissing");

	CommandResult keyResult =
		ctx.dispatch(taro.fd, makeMessage("MODE", "#modemissing", "+k"));
	CommandResult limitResult =
		ctx.dispatch(taro.fd, makeMessage("MODE", "#modemissing", "+l"));
	CommandResult opResult =
		ctx.dispatch(taro.fd, makeMessage("MODE", "#modemissing", "+o"));
	CommandResult deopResult =
		ctx.dispatch(taro.fd, makeMessage("MODE", "#modemissing", "-o"));

	EXPECT_CONTAINS(keyResult.replies[0].message, " 461 ");
	EXPECT_CONTAINS(limitResult.replies[0].message, " 461 ");
	EXPECT_CONTAINS(opResult.replies[0].message, " 461 ");
	EXPECT_CONTAINS(deopResult.replies[0].message, " 461 ");
  }

  void testModeQueryReturns324WithModesAndArgs() {
	TestContext ctx;
	TestClient taro = ctx.registerClient(87, "taro");
	ctx.join(taro.fd, "#modequery");
	ctx.dispatch(taro.fd, makeMessage("MODE", "#modequery", "+i"));
	ctx.dispatch(taro.fd, makeMessage("MODE", "#modequery", "+t"));
	ctx.dispatch(taro.fd, makeMessage("MODE", "#modequery", "+k", "secret"));
	ctx.dispatch(taro.fd, makeMessage("MODE", "#modequery", "+l", "2"));

	CommandResult result =
		ctx.dispatch(taro.fd, makeMessage("MODE", "#modequery"));

	EXPECT_EQ(static_cast<size_t>(1), result.replies.size());
	EXPECT_EQ(taro.fd, result.replies[0].fd);
	EXPECT_CONTAINS(result.replies[0].message,
					" 324 taro #modequery +itkl secret 2");
  }

  void testJoinFullChannelReturns471WithoutAddingClient() {
	TestContext ctx;
	TestClient taro = ctx.registerClient(88, "taro");
	TestClient hanako = ctx.registerClient(89, "hanako");
	ctx.join(taro.fd, "#full");
	ctx.dispatch(taro.fd, makeMessage("MODE", "#full", "+l", "1"));

	CommandResult result = ctx.join(hanako.fd, "#full");

	EXPECT_EQ(static_cast<size_t>(1), result.replies.size());
	EXPECT_EQ(hanako.fd, result.replies[0].fd);
	EXPECT_CONTAINS(result.replies[0].message, " 471 hanako #full ");
	EXPECT_FALSE(ctx.state.getChannel("#full")->hasMember(ctx.client(hanako.fd)));
  }

  void testJoinKeyedChannelReturns475UntilKeyMatches() {
	TestContext ctx;
	TestClient taro = ctx.registerClient(90, "taro");
	TestClient hanako = ctx.registerClient(91, "hanako");
	ctx.join(taro.fd, "#keyed");
	ctx.dispatch(taro.fd, makeMessage("MODE", "#keyed", "+k", "secret"));

	CommandResult missing = ctx.join(hanako.fd, "#keyed");
	CommandResult wrong =
		ctx.dispatch(hanako.fd, makeMessage("JOIN", "#keyed", "wrong"));
	CommandResult correct =
		ctx.dispatch(hanako.fd, makeMessage("JOIN", "#keyed", "secret"));

	EXPECT_CONTAINS(missing.replies[0].message, " 475 hanako #keyed ");
	EXPECT_CONTAINS(wrong.replies[0].message, " 475 hanako #keyed ");
	EXPECT_FALSE(resultHasReplyToContaining(correct, hanako.fd, " 475 "));
	EXPECT_TRUE(ctx.state.getChannel("#keyed")->hasMember(ctx.client(hanako.fd)));
	EXPECT_REPLY_TO_CONTAINS(correct, hanako.fd,
							 ":hanako!user@client.example JOIN #keyed");
  }

  void testInvalidJoinChannelReturns403WithoutCreatingChannel() {
	TestContext ctx;
	TestClient taro = ctx.registerClient(92, "taro");

	CommandResult noPrefix = ctx.join(taro.fd, "channel");
	CommandResult comma = ctx.join(taro.fd, "#bad,name");

	EXPECT_EQ(static_cast<size_t>(1), noPrefix.replies.size());
	EXPECT_CONTAINS(noPrefix.replies[0].message, " 403 ");
	EXPECT_TRUE(ctx.state.getChannel("channel") == NULL);
	EXPECT_EQ(static_cast<size_t>(1), comma.replies.size());
	EXPECT_CONTAINS(comma.replies[0].message, " 403 ");
	EXPECT_TRUE(ctx.state.getChannel("#bad,name") == NULL);
  }

  void testInvalidChannelNamesReturn403AcrossChannelCommands() {
	TestContext ctx;
	TestClient taro = ctx.registerClient(93, "taro");
	TestClient hanako = ctx.registerClient(94, "hanako");
	ctx.join(taro.fd, "#valid");
	ctx.join(hanako.fd, "#valid");

	CommandResult part = ctx.dispatch(taro.fd, makeMessage("PART", "valid"));
	CommandResult privmsg =
		ctx.dispatch(taro.fd, makeMessage("PRIVMSG", "#bad,name", "hello"));
	CommandResult kick =
		ctx.dispatch(taro.fd, makeMessage("KICK", "#bad,name", hanako.nick));
	CommandResult invite =
		ctx.dispatch(taro.fd, makeMessage("INVITE", hanako.nick, "#bad,name"));
	CommandResult invite_both_invalid =
		ctx.dispatch(taro.fd, makeMessage("INVITE", "1bad", "#bad,name"));
	CommandResult topic =
		ctx.dispatch(taro.fd, makeMessage("TOPIC", "#bad,name", "topic"));
	CommandResult mode =
		ctx.dispatch(taro.fd, makeMessage("MODE", "#bad,name", "+i"));

	EXPECT_CONTAINS(part.replies[0].message, " 403 ");
	EXPECT_CONTAINS(privmsg.replies[0].message, " 403 ");
	EXPECT_CONTAINS(kick.replies[0].message, " 403 ");
	EXPECT_CONTAINS(invite.replies[0].message, " 403 ");
	EXPECT_CONTAINS(topic.replies[0].message, " 403 ");
	EXPECT_CONTAINS(mode.replies[0].message, " 403 ");
	EXPECT_CONTAINS(invite_both_invalid.replies[0].message, " 401 ");
	EXPECT_TRUE(ctx.state.getChannel("#bad,name") == NULL);
	EXPECT_TRUE(ctx.state.getChannel("#valid")->hasMember(ctx.client(taro.fd)));
	EXPECT_TRUE(ctx.state.getChannel("#valid")->hasMember(ctx.client(hanako.fd)));
  }

  void testNoticeToInvalidTargetReturnsNoReplyAndNoMutation() {
	TestContext ctx;
	TestClient taro = ctx.registerClient(95, "taro");
	ctx.join(taro.fd, "#valid");

	CommandResult invalidChannel =
		ctx.dispatch(taro.fd, makeMessage("NOTICE", "#bad,name", "hello"));
	CommandResult invalidNick =
		ctx.dispatch(taro.fd, makeMessage("NOTICE", "1bad", "hello"));

	EXPECT_EQ(static_cast<size_t>(0), invalidChannel.replies.size());
	EXPECT_EQ(static_cast<size_t>(0), invalidNick.replies.size());
	EXPECT_TRUE(ctx.state.getChannel("#bad,name") == NULL);
	EXPECT_TRUE(ctx.state.getClientByNick("1bad") == NULL);
  }

  void testInvalidNickOperandsReturn401() {
	TestContext ctx;
	TestClient taro = ctx.registerClient(96, "taro");
	TestClient hanako = ctx.registerClient(97, "hanako");
	ctx.join(taro.fd, "#ops");
	ctx.join(hanako.fd, "#ops");

	CommandResult kick =
		ctx.dispatch(taro.fd, makeMessage("KICK", "#ops", "#bad"));
	CommandResult invite =
		ctx.dispatch(taro.fd, makeMessage("INVITE", "#bad", "#ops"));
	CommandResult mode =
		ctx.dispatch(taro.fd, makeMessage("MODE", "#ops", "+o", "#bad"));
	CommandResult privmsg =
		ctx.dispatch(taro.fd, makeMessage("PRIVMSG", "1bad", "hello"));

	EXPECT_CONTAINS(kick.replies[0].message, " 401 ");
	EXPECT_CONTAINS(invite.replies[0].message, " 401 ");
	EXPECT_CONTAINS(mode.replies[0].message, " 401 ");
	EXPECT_CONTAINS(privmsg.replies[0].message, " 401 ");
	EXPECT_TRUE(ctx.state.getClientByNick("#bad") == NULL);
	EXPECT_TRUE(ctx.state.getClientByNick("1bad") == NULL);
  }

  void testConnectionHealthMonitorGeneratesPingAndTimesOut() {
	ConnectionHealthMonitor monitor(10);

	CommandResult result = monitor.generatePing(100, 1000);

	EXPECT_EQ(static_cast<size_t>(1), result.replies.size());
	EXPECT_EQ(100, result.replies[0].fd);
	EXPECT_CONTAINS(result.replies[0].message, " PING ");
	EXPECT_CONTAINS(result.replies[0].message, "irc.local-100-1000");
	EXPECT_TRUE(monitor.isWaitingForPong(100));
	EXPECT_EQ(std::string("irc.local-100-1000"),
			  monitor.getExpectedPongToken(100));
	EXPECT_FALSE(monitor.hasTimedOut(100, 1010));
	EXPECT_TRUE(monitor.hasTimedOut(100, 1011));
  }

  void testConnectionHealthMonitorPongMatching() {
	ConnectionHealthMonitor monitor(10);
	monitor.generatePing(101, 2000);

	monitor.markPongReceived(101, "wrong", 2001);
	EXPECT_TRUE(monitor.isWaitingForPong(101));

	monitor.markPongReceived(101, "irc.local-101-2000", 2002);
	EXPECT_FALSE(monitor.isWaitingForPong(101));
	EXPECT_FALSE(monitor.hasTimedOut(101, 3000));
  }

  void testConnectionHealthMonitorCollectsTimedOutClientsOnly() {
	ConnectionHealthMonitor monitor(5);

	monitor.generatePing(100, 1000);
	monitor.generatePing(101, 1000);
	monitor.markPongReceived(101, "irc.local-101-1000", 1001);

	std::vector<int> timedOut = monitor.collectTimedOutClients(1006);
	EXPECT_EQ(static_cast<size_t>(1), timedOut.size());
	EXPECT_EQ(100, timedOut[0]);
	EXPECT_TRUE(monitor.isWaitingForPong(100));
  }

  void testConnectionHealthMonitorRemovesClientState() {
	ConnectionHealthMonitor monitor(10);
	monitor.generatePing(102, 4000);

	monitor.removeClient(102);

	EXPECT_FALSE(monitor.isWaitingForPong(102));
	EXPECT_FALSE(monitor.hasTimedOut(102, 5000));
	EXPECT_EQ(std::string(""), monitor.getExpectedPongToken(102));
  }

  void testPongUpdatesHealthMonitor() {
	TestContext ctx;
	ConnectionHealthMonitor monitor(10);
	TestClient taro = ctx.addClient(63);
	monitor.generatePing(taro.fd, 3000);

	CommandResult result =
		ctx.dispatchWithHealth(taro.fd,
							   makeMessage("PONG", "irc.local-63-3000"),
							   monitor);

	EXPECT_FALSE(result.shouldDisconnect);
	EXPECT_EQ(static_cast<size_t>(0), result.replies.size());
	EXPECT_FALSE(monitor.isWaitingForPong(taro.fd));
  }

  void testPongUsesLastParamAsToken() {
	TestContext ctx;
	ConnectionHealthMonitor monitor(10);
	TestClient taro = ctx.addClient(66);
	monitor.generatePing(taro.fd, 3000);

	CommandResult result =
		ctx.dispatchWithHealth(taro.fd,
							   makeMessage("PONG", "irc.local",
										   "irc.local-66-3000"),
							   monitor);

	EXPECT_FALSE(result.shouldDisconnect);
	EXPECT_EQ(static_cast<size_t>(0), result.replies.size());
	EXPECT_FALSE(monitor.isWaitingForPong(taro.fd));
  }

  void testPongWithoutHealthMonitorIsNoOp() {
	TestContext ctx;
	TestClient taro = ctx.addClient(64);

	CommandResult result = ctx.dispatch(taro.fd, makeMessage("PONG", "token"));

	EXPECT_FALSE(result.shouldDisconnect);
	EXPECT_EQ(static_cast<size_t>(0), result.replies.size());
  }

  void testPongWithoutTokenReturnsNeedMoreParams() {
	TestContext ctx;
	TestClient taro = ctx.addClient(65);

	CommandResult result = ctx.dispatch(taro.fd, makeMessageNoParams("PONG"));

	EXPECT_FALSE(result.shouldDisconnect);
	EXPECT_EQ(static_cast<size_t>(1), result.replies.size());
	EXPECT_EQ(taro.fd, result.replies[0].fd);
	EXPECT_CONTAINS(result.replies[0].message, " 461 ");
	EXPECT_CONTAINS(result.replies[0].message, "PONG");
  }

  void testDisconnectNotifierBuildsQuitNotificationOnly() {
	TestContext ctx;
	DisconnectNotifier notifier;
	TestClient taro = ctx.registerClient(60, "taro");
	TestClient hanako = ctx.registerClient(61, "hanako");
	TestClient jiro = ctx.registerClient(62, "jiro");
	ctx.join(taro.fd, "#room");
	ctx.join(hanako.fd, "#room");
	ctx.join(taro.fd, "#shared2");
	ctx.join(hanako.fd, "#shared2");
	ctx.join(jiro.fd, "#other");

	CommandResult result =
		notifier.build(DisconnectEvent(taro.fd, "Inactivity timeout"),
					   ctx.state);

	EXPECT_EQ(static_cast<size_t>(1), result.replies.size());
	EXPECT_EQ(hanako.fd, result.replies[0].fd);
	EXPECT_CONTAINS(result.replies[0].message,
					":taro!user@client.example QUIT :Inactivity timeout");
	EXPECT_TRUE(ctx.state.getClientByFd(taro.fd) != NULL);
	EXPECT_TRUE(ctx.state.getChannel("#room") != NULL);
	EXPECT_TRUE(ctx.state.getChannel("#shared2") != NULL);
	EXPECT_TRUE(ctx.state.getChannel("#other") != NULL);
	ctx.state.removeClient(taro.fd);
	EXPECT_TRUE(ctx.state.getClientByFd(taro.fd) == NULL);
  }

}  // namespace

int main() {
  runTest("parser basic message", testParserBasicMessage);
  runTest("parser full prefix", testParserFullPrefix);
  runTest("parser server numeric prefix", testParserServerNumericPrefix);
  runTest("parser prefix only is empty message",
          testParserPrefixOnlyIsEmptyMessage);
  runTest("ping returns pong", testPingReturnsPong);
  runTest("dispatch unknown fd returns empty result",
		  testDispatchUnknownFdReturnsEmptyResult);
  runTest("ping without param uses star target before nick",
		  testPingWithoutParamUsesStarTargetBeforeNick);
  runTest("unknown command uses star target before nick",
		  testUnknownCommandUsesStarTargetBeforeNick);
  runTest("unknown command uses nick target after nick",
		  testUnknownCommandUsesNickTargetAfterNick);
  runTest("registration flow uses real C state",
		  testRegistrationFlowUsesRealCState);
  runTest("nick before pass is rejected", testNickBeforePassIsRejected);
  runTest("user before pass is rejected", testUserBeforePassIsRejected);
  runTest("nick conflict returns numeric", testNickConflictReturnsNumeric);
  runTest("invalid nicknames return 432 without updating state",
		  testInvalidNicknamesReturn432WithoutUpdatingState);
  runTest("valid nickname boundaries are accepted",
		  testValidNicknameBoundariesAreAccepted);

  runTest("not registered reply format", testNotRegisteredReplyFormat);

  runTest("join before registration returns 451",
		  testJoinBeforeRegistrationReturns451);
  runTest("join before registration uses nick target after nick",
		  testJoinBeforeRegistrationUsesNickTargetAfterNick);

  runTest("join after registration echoes to self",
			testJoinAfterRegistrationEchoesToSelf);
  runTest("join success sends topic names and no 999",
			testJoinSuccessSendsTopicNamesAndNo999);
  runTest("join names marks operators",
			testJoinNamesMarksOperators);
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
  runTest("invite delegates and preserves behavior",
			testInviteDelegatesAndPreservesBehavior);
  runTest("topic delegates and preserves behavior",
			testTopicDelegatesAndPreservesBehavior);
  runTest("kick success broadcasts and removes target",
			testKickSuccessBroadcastsAndRemovesTarget);
  runTest("kick non-operator returns 482",
			testKickNonOperatorReturns482);
  runTest("kick missing target returns 461",
			testKickMissingTargetReturns461);
  runTest("kick target not in channel returns 441",
			testKickTargetNotInChannelReturns441);
  runTest("mode flags key and limit mutate state",
			testModeFlagsKeyAndLimitMutateState);
  runTest("mode operator mutates state",
			testModeOperatorMutatesState);
  runTest("mode compound tokens return error without mutation",
			testModeCompoundTokensReturnErrorWithoutMutation);
  runTest("mode missing arguments return 461",
			testModeMissingArgumentsReturn461);
  runTest("mode query returns 324 with modes and args",
			testModeQueryReturns324WithModesAndArgs);
  runTest("join full channel returns 471 without adding client",
			testJoinFullChannelReturns471WithoutAddingClient);
  runTest("join keyed channel returns 475 until key matches",
			testJoinKeyedChannelReturns475UntilKeyMatches);
  runTest("invalid join channel returns 403 without creating channel",
			testInvalidJoinChannelReturns403WithoutCreatingChannel);
  runTest("invalid channel names return 403 across channel commands",
			testInvalidChannelNamesReturn403AcrossChannelCommands);
  runTest("notice to invalid target returns no reply and no mutation",
			testNoticeToInvalidTargetReturnsNoReplyAndNoMutation);
  runTest("invalid nick operands return 401",
			testInvalidNickOperandsReturn401);
  runTest("connection health monitor generates ping and times out",
			testConnectionHealthMonitorGeneratesPingAndTimesOut);
  runTest("connection health monitor pong matching",
			testConnectionHealthMonitorPongMatching);
  runTest("connection health monitor collects timed out clients only",
			testConnectionHealthMonitorCollectsTimedOutClientsOnly);
  runTest("connection health monitor removes client state",
			testConnectionHealthMonitorRemovesClientState);
  runTest("pong updates health monitor",
			testPongUpdatesHealthMonitor);
  runTest("pong uses last param as token",
			testPongUsesLastParamAsToken);
  runTest("pong without health monitor is no-op",
			testPongWithoutHealthMonitorIsNoOp);
  runTest("pong without token returns need more params",
			testPongWithoutTokenReturnsNeedMoreParams);
  runTest("disconnect notifier builds quit notification only",
			testDisconnectNotifierBuildsQuitNotificationOnly);

  std::cout << "Assertions passed: " << g_passed << std::endl;
  if (g_failed != 0) {
	std::cout << "Assertions failed: " << g_failed << std::endl;
	return 1;
  }
  return 0;
}
