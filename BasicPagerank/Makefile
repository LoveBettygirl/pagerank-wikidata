NAME = pagerank
BUILD_DIR ?= .
OBJ_DIR ?= $(BUILD_DIR)/obj
BINARY ?= $(BUILD_DIR)/$(NAME)

.DEFAULT_GOAL = app

# Compilation flags
CXX = g++
LD = g++
CXXFLAGS   += -O2 -MMD -std=c++11

# Files to be compiled
SRCS = $(shell find ./ -name "*.cpp")
OBJS = $(SRCS:./%.cpp=$(OBJ_DIR)/%.o)

# Compilation patterns
$(OBJ_DIR)/%.o: ./%.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

# Depencies
-include $(OBJS:.o=.d)

# Some convinient rules

.PHONY: app run clean
app: $(BINARY)


# Execute
EXEC := $(BINARY) $(ARGS)

$(BINARY): $(OBJS)
	@$(LD) -O2 -o $@ $^

run: $(BINARY)
	$(EXEC)

clean: 
	rm -rf $(OBJ_DIR)
	rm -rf $(BUILD_DIR)/$(NAME)
