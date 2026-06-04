NAME        = ircserv

CXX         = c++
CXXFLAGS    = -Wall -Wextra -Werror -std=c++98
INCLUDES    = -I./include

SRCS_LOGIC  = 
SRCS_NET    = 
SRCS_PARSER = 

SRCS_MAIN   = src/main.cpp

OBJS        = $(SRCS_LOGIC:.cpp=.o) $(SRCS_NET:.cpp=.o) $(SRCS_PARSER:.cpp=.o) $(SRCS_MAIN:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS)
	$(MAKE) -C tests/clayer clean

fclean: clean
	rm -f $(NAME)
	$(MAKE) -C tests/clayer fclean

re: fclean all

test:
	$(MAKE) -C tests/clayer run

.PHONY: all clean fclean re test