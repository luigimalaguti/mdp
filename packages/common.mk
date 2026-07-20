ifeq ($(origin CC),default)
CC				:= gcc
endif
ifeq ($(origin CXX),default)
CXX				:= g++
endif

CFLAGS			?= -std=c99   -g -Wall -Wextra -Werror -Wpedantic
CXXFLAGS		?= -std=c++23 -g -Wall -Wextra -Werror -Wpedantic

LDFLAGS			?=

SRC_DIR			?= src
INC_DIR			?= include
BUILD_DIR		?= build
BIN_DIR			?= bin
EXEC			?= main
TARGET			?= $(BIN_DIR)/$(EXEC)

SRCS_C			:= $(shell find $(SRC_DIR) -name '*.c'   2>/dev/null)
SRCS_CXX		:= $(shell find $(SRC_DIR) -name '*.cpp' 2>/dev/null)
SRCS			:= $(SRCS_C) $(SRCS_CXX)

OBJS			:= $(addprefix $(BUILD_DIR)/,$(addsuffix .o,$(SRCS)))
DEPS			:= $(OBJS:.o=.d)

LD				:= $(if $(strip $(SRCS_CXX)),$(CXX),$(CC))

CPPFLAGS		+= -MMD -MP -I$(INC_DIR)

COMP_DB			:= compile_commands.json
CACHE_DIR		:= .cache

.DEFAULT_GOAL	:= all

.PHONY: all
all: rebuild

.PHONY: build
build: $(TARGET) $(COMP_DB)

.PHONY: rebuild
rebuild: clean build

$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(LD) $^ -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(BUILD_DIR)/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR) $(COMP_DB) $(CACHE_DIR)

.PHONY: $(COMP_DB)
$(COMP_DB):
	@{ \
	    sep='['; \
	    for f in $(SRCS_C); do \
	        printf '%s\n  {"directory": "%s", "file": "%s", "command": "$(CC) $(CFLAGS) $(CPPFLAGS) -c %s"}' "$$sep" "$$(pwd)" "$$f" "$$f"; \
	        sep=','; \
	    done; \
	    for f in $(SRCS_CXX); do \
	        printf '%s\n  {"directory": "%s", "file": "%s", "command": "$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c %s"}' "$$sep" "$$(pwd)" "$$f" "$$f"; \
	        sep=','; \
	    done; \
	    printf '\n]\n'; \
	} > $(COMP_DB)

-include $(DEPS)
