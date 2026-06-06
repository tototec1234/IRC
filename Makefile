NAME        = ircserv

CXX         = c++
CXXFLAGS    = -Wall -Wextra -Werror -std=c++98
INCLUDES    = -I./include

SRCS_A      = src/a/Server.cpp \
              src/a/Connection.cpp

SRCS_B      = src/b/Message.cpp \
              src/b/Parser.cpp \
              src/b/CommandResult.cpp \
              src/b/ReplyBuilder.cpp \
              src/b/CommandDispatcher.cpp

SRCS_C      = src/c/ServerState.cpp \
              src/c/Client.cpp \
              src/c/ClientRegistry.cpp \
              src/c/Channel.cpp \
              src/c/ChannelModes.cpp \
              src/c/Utils.cpp

SRCS_MAIN   = src/main.cpp

OBJS        = $(SRCS_A:.cpp=.o) $(SRCS_B:.cpp=.o) \
              $(SRCS_C:.cpp=.o) $(SRCS_MAIN:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS)
	$(MAKE) -C tests/clayer clean
	$(MAKE) -C tests/blayer clean

fclean: clean
	rm -f $(NAME)
	$(MAKE) -C tests/clayer fclean
	$(MAKE) -C tests/blayer fclean

re: fclean all

test:
	$(MAKE) -C tests/clayer run
	$(MAKE) -C tests/blayer run

.PHONY: all clean fclean re test