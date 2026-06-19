#include <iostream>
#include <string>
#include <vector>

#include "c/Client.hpp"
#include "b/CommandDispatcher.hpp"
#include "b/CommandResult.hpp"
#include "b/Message.hpp"
#include "b/Parser.hpp"
#include "b/ReplyBuilder.hpp" //torinoue
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

void addClient(ServerState& state, int fd) {
  state.addClient(fd, "client.example");
}

Message makeUserMessage() {
  std::vector<std::string> params;
  params.push_back("user");
  params.push_back("0");
  params.push_back("*");
  params.push_back("Real Name");
  return Message("USER", params);
}

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
  ServerState state("pw");
  addClient(state, 10);
  CommandDispatcher dispatcher;

  CommandResult result =
	  dispatcher.dispatch(10, makeMessage("PING", "token"), state);

  EXPECT_FALSE(result.shouldDisconnect);
  EXPECT_EQ(static_cast<size_t>(1), result.replies.size());
  EXPECT_EQ(10, result.replies[0].fd);
  EXPECT_CONTAINS(result.replies[0].message, " PONG ");
  EXPECT_CONTAINS(result.replies[0].message, "token");
}

void testRegistrationFlowUsesRealCState() {
  ServerState state("pw");
  addClient(state, 20);
  Client* client = state.getClientByFd(20);
  CommandDispatcher dispatcher;

  EXPECT_TRUE(client != NULL);
  CommandResult passResult =
	  dispatcher.dispatch(20, makeMessage("PASS", "pw"), state);
  CommandResult nickResult =
	  dispatcher.dispatch(20, makeMessage("NICK", "taro"), state);
  CommandResult userResult = dispatcher.dispatch(20, makeUserMessage(), state);

  EXPECT_EQ(static_cast<size_t>(0), passResult.replies.size());
  EXPECT_EQ(static_cast<size_t>(0), nickResult.replies.size());
  EXPECT_TRUE(client->isPassOk());
  EXPECT_EQ(std::string("taro"), client->getNick());
  EXPECT_EQ(std::string("taro!user@client.example"), client->getFullPrefix());
  EXPECT_EQ(client, state.getClientByNick("taro"));
  EXPECT_TRUE(client->isRegistered());
  EXPECT_EQ(static_cast<size_t>(1), userResult.replies.size());
  EXPECT_CONTAINS(userResult.replies[0].message, " 001 taro ");
}

void testNickBeforePassIsRejected() {
  ServerState state("pw");
  addClient(state, 25);
  Client* client = state.getClientByFd(25);
  CommandDispatcher dispatcher;

  CommandResult result =
	  dispatcher.dispatch(25, makeMessage("NICK", "taro"), state);

  EXPECT_TRUE(client != NULL);
  EXPECT_FALSE(client->isPassOk());
  EXPECT_TRUE(client->getNick().empty());
  EXPECT_TRUE(state.getClientByNick("taro") == NULL);
  EXPECT_FALSE(client->isRegistered());
  EXPECT_EQ(static_cast<size_t>(1), result.replies.size());
  EXPECT_CONTAINS(result.replies[0].message, " 464 ");
}

void testUserBeforePassIsRejected() {
  ServerState state("pw");
  addClient(state, 26);
  Client* client = state.getClientByFd(26);
  CommandDispatcher dispatcher;

  CommandResult result = dispatcher.dispatch(26, makeUserMessage(), state);

  EXPECT_TRUE(client != NULL);
  EXPECT_FALSE(client->isPassOk());
  EXPECT_TRUE(client->getUsername().empty());
  EXPECT_TRUE(client->getRealname().empty());
  EXPECT_FALSE(client->isRegistered());
  EXPECT_EQ(static_cast<size_t>(1), result.replies.size());
  EXPECT_CONTAINS(result.replies[0].message, " 464 ");
}

void testNickConflictReturnsNumeric() {
  ServerState state("pw");
  addClient(state, 30);
  addClient(state, 31);
  CommandDispatcher dispatcher;

  dispatcher.dispatch(30, makeMessage("PASS", "pw"), state);
  dispatcher.dispatch(31, makeMessage("PASS", "pw"), state);
  dispatcher.dispatch(30, makeMessage("NICK", "taro"), state);
  CommandResult result =
	  dispatcher.dispatch(31, makeMessage("NICK", "TARO"), state);

  EXPECT_EQ(static_cast<size_t>(1), result.replies.size());
  EXPECT_CONTAINS(result.replies[0].message, " 433 ");
  EXPECT_TRUE(state.getClientByFd(31)->getNick().empty());
}

/* test tuika*/
void testNotRegisteredReplyFormat() {
  ServerState state("pw");
  addClient(state, 40);
// Client* client = state.getClientByFd(26);
Client* client = state.getClientByFd(40);
//   CommandDispatcher dispatcher;

//   dispatcher.dispatch(30, makeMessage("PASS", "pw"), state);
//   dispatcher.dispatch(31, makeMessage("PASS", "pw"), state);
//   dispatcher.dispatch(30, makeMessage("NICK", "taro"), state);
//   CommandResult result =
//       dispatcher.dispatch(31, makeMessage("NICK", "TARO"), state);

	std::string reply = ReplyBuilder::noRegistered(*client);

//   EXPECT_EQ(static_cast<size_t>(1), result.replies.size());
//   EXPECT_CONTAINS(result.replies[0].message, " 433 ");
//   EXPECT_TRUE(state.getClientByFd(31)->getNick().empty());
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

}  // namespace

void testJoinBeforeRegistrationReturns451() {
	ServerState state("pw");
	addClient(state, 42);
	Client* client = state.getClientByFd(42);
	CommandDispatcher dispatcher;
  
	CommandResult result =
		dispatcher.dispatch(42, makeMessage("JOIN", "#room42Tokyo"), state);
  
	EXPECT_FALSE(client->isRegistered());
	EXPECT_EQ(static_cast<size_t>(1), result.replies.size());
	EXPECT_EQ(42, result.replies[0].fd);
	EXPECT_CONTAINS(result.replies[0].message, " 451 ");
  }


  void testJoinAfterRegistrationEchoesToSelf() {
	ServerState state("pw");
	addClient(state, 43);
	CommandDispatcher dispatcher;
	dispatcher.dispatch(43, makeMessage("PASS", "pw"), state);
	dispatcher.dispatch(43, makeMessage("NICK", "taro"), state);
	dispatcher.dispatch(43, makeUserMessage(), state);
	CommandResult result =
		dispatcher.dispatch(43, makeMessage("JOIN", "#taros_room"), state);
	EXPECT_TRUE(state.getClientByFd(43)->isRegistered());
	EXPECT_TRUE(result.replies.size() >= static_cast<size_t>(1));
	EXPECT_EQ(43, result.replies[0].fd);
	EXPECT_CONTAINS(result.replies[0].message, "JOIN" );
	EXPECT_CONTAINS(result.replies[0].message, ":taro!user@client.example JOIN #taros_room" );
	// std::cout << std::endl << result.replies[0].message << std::endl;
  }

  void testPartSingleMemberEchoesToSelf() {
	ServerState state("pw");
	addClient(state, 45);
	CommandDispatcher dispatcher;
	dispatcher.dispatch(45, makeMessage("PASS", "pw"), state);
	dispatcher.dispatch(45, makeMessage("NICK", "taro"), state);
	dispatcher.dispatch(45, makeUserMessage(), state);
	dispatcher.dispatch(45, makeMessage("JOIN", "#solo"), state);

	CommandResult result =
		dispatcher.dispatch(45, makeMessage("PART", "#solo"), state);

	EXPECT_EQ(static_cast<size_t>(1), result.replies.size());
	EXPECT_EQ(45, result.replies[0].fd);
	EXPECT_CONTAINS(result.replies[0].message,
					":taro!user@client.example PART #solo");
	EXPECT_TRUE(state.getChannel("#solo") == NULL);
  }

  void testQuitBeforeRegistrationDisconnects() {
	ServerState state("pw");
	addClient(state, 44);
	CommandDispatcher dispatcher;

	CommandResult result =
		dispatcher.dispatch(44, makeMessage("QUIT", "bye"), state);

	EXPECT_TRUE(result.shouldDisconnect);
	EXPECT_EQ(static_cast<size_t>(0), result.replies.size());
  }

#include <stdio.h>

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

			// printf("------%d------------", __LINE__ );

  std::cout << "Assertions passed: " << g_passed << std::endl;
  if (g_failed != 0) {
	std::cout << "Assertions failed: " << g_failed << std::endl;
	return 1;
  }
  return 0;
}
