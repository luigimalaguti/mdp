# ==============================
# Compiler and flags
# ==============================
CXX      := g++
CC       := gcc
CXXFLAGS := -std=c++23 -g -Wall -Wextra -Werror -Wpedantic
CFLAGS   := -std=c99   -g -Wall -Wextra -Werror -Wpedantic
LDFLAGS  := # i.e. -lm -lpthread

# ==============================
# Directories
# ==============================
SRC_DIR   := src
INC_DIR   := include
BUILD_DIR := build
BIN_DIR   := bin

# ==============================
# Sources and objects
# ==============================
SRCS_CXX := $(wildcard $(SRC_DIR)/*.cpp)
SRCS_C   := $(wildcard $(SRC_DIR)/*.c)

OBJS_CXX := $(SRCS_CXX:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.cpp.o)
OBJS_C   := $(SRCS_C:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.c.o)

BINS_CXX := $(SRCS_CXX:$(SRC_DIR)/%.cpp=$(BIN_DIR)/%_cpp)
BINS_C   := $(SRCS_C:$(SRC_DIR)/%.c=$(BIN_DIR)/%_c)

# ==============================
# Main rule
# ==============================
.DEFAULT_GOAL := all

.PHONY: all
all: $(BINS_CXX) $(BINS_C)

# ==============================
# C++ link
# ==============================
$(BIN_DIR)/%_cpp: $(BUILD_DIR)/%.cpp.o
	@mkdir -p $(BIN_DIR)
	$(CXX) $< -o $@ $(LDFLAGS)

# ==============================
# C link
# ==============================
$(BIN_DIR)/%_c: $(BUILD_DIR)/%.c.o
	@mkdir -p $(BIN_DIR)
	$(CC) $< -o $@ $(LDFLAGS)

# ==============================
# C++ compilation
# ==============================
$(BUILD_DIR)/%.cpp.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

# ==============================
# C compilation
# ==============================
$(BUILD_DIR)/%.c.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

# ==============================
# Utility
# ==============================
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

.PHONY: rebuild
rebuild: clean all
