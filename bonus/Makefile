NAME        = ircbot

CXX         = c++
CXXFLAGS    = -Wall -Wextra -Werror -std=c++98
INCLUDES    = -I. -I../include

SRCS        = main.cpp \
              Bot.cpp \
              irc/LineBuffer.cpp \
              irc/IrcFormatter.cpp

B_SRCS      = ../src/b/Message.cpp \
              ../src/b/Parser.cpp

OBJS        = $(SRCS:.cpp=.o)
B_OBJS      = $(B_SRCS:../src/b/%.cpp=b_obj/%.o)

all: $(NAME)

$(NAME): $(OBJS) $(B_OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

b_obj/%.o: ../src/b/%.cpp
	mkdir -p b_obj
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(B_OBJS)

fclean: clean
	rm -f $(NAME)

re:
	$(MAKE) fclean
	$(MAKE) all

.PHONY: all clean fclean re
