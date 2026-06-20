TARGET      = test_runner

CXX         = c++
CXXFLAGS    = -Wall -Wextra -Werror -std=c++98

INCLUDES    = -I../../include -I.

SRC_C_DIR   = ../../src/c
SRC_B_DIR   = ../../src/b
SRC_LIFECYCLE_DIR = ../../src/lifecycle

SRCS_C      = $(SRC_C_DIR)/ServerState.cpp \
              $(SRC_C_DIR)/Client.cpp \
              $(SRC_C_DIR)/ClientRegistry.cpp \
              $(SRC_C_DIR)/Channel.cpp \
              $(SRC_C_DIR)/ChannelModes.cpp \
              $(SRC_C_DIR)/Utils.cpp

SRCS_B      = $(SRC_B_DIR)/Message.cpp \
              $(SRC_B_DIR)/Parser.cpp \
              $(SRC_B_DIR)/CommandResult.cpp \
              $(SRC_B_DIR)/ReplyBuilder.cpp \
              $(SRC_B_DIR)/CommandDispatcher.cpp \
              $(SRC_B_DIR)/DisconnectEvent.cpp \
              $(SRC_B_DIR)/DisconnectNotifier.cpp

SRCS_LIFECYCLE = $(SRC_LIFECYCLE_DIR)/ConnectionHealthMonitor.cpp

SRCS_TEST   = test_main.cpp

OBJS_C      = $(notdir $(SRCS_C:.cpp=.o))
OBJS_B      = $(notdir $(SRCS_B:.cpp=.o))
OBJS_LIFECYCLE = $(notdir $(SRCS_LIFECYCLE:.cpp=.o))
OBJS_TEST   = $(SRCS_TEST:.cpp=.o)
OBJS        = $(OBJS_C) $(OBJS_B) $(OBJS_LIFECYCLE) $(OBJS_TEST)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@

$(OBJS_C): %.o: $(SRC_C_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJS_B): %.o: $(SRC_B_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJS_LIFECYCLE): %.o: $(SRC_LIFECYCLE_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

run: all
	@echo "=================================================="
	@echo " Running B Layer Dispatcher Boundary Tests... "
	@echo "=================================================="
	./$(TARGET)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(TARGET)

re: fclean all

.PHONY: all clean fclean re run
