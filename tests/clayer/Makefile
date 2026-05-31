TARGET      = test_runner

CXX         = c++
CXXFLAGS    = -Wall -Wextra -Werror -std=c++98

INCLUDES    = -I../../include -I.

SRC_DIR     = ../../src/logic
SRCS_LOGIC  = $(SRC_DIR)/ServerState.cpp \
              $(SRC_DIR)/Client.cpp \


SRCS_TEST   = test_main.cpp \

OBJS_LOGIC  = $(notdir $(SRCS_LOGIC:.cpp=.o))
OBJS_TEST   = $(SRCS_TEST:.cpp=.o)
OBJS        = $(OBJS_LOGIC) $(OBJS_TEST)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@

$(OBJS_LOGIC): %.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

run: all
	@echo "=================================================="
	@echo " Running C Layer Unit Tests... "
	@echo "=================================================="
	./$(TARGET)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(TARGET)

re: fclean all

.PHONY: all clean fclean re run