# ============================================================
# common.mk — universal build logic for every program folder.
#
# Each program's Makefile contains just:
#
#     include ../common.mk
#
# Layout convention (per program folder):
#     src/        sources: *.c and/or *.cpp, subfolders allowed
#     include/    headers (optional)
#     test/       test sources (optional): EACH file becomes its
#                 own executable bin/<name>, linked against the
#                 library objects and the external libraries
#     external/   vendored libraries (optional): for each
#                 external/<lib>/, src/ is compiled in and
#                 include/ is added to the header search path
#     build/      objects + dependency files (generated)
#     bin/        binaries (generated)
#
# Program vs library: if a src/ file defines a main function
# (written literally as "int main("), the project is a program
# and `all` produces bin/main — the fixed name .zed/debug.json
# relies on. Otherwise it is a library and `all` produces the
# test executables only. A program with tests works too: the
# file containing main is excluded from the test link.
#
# Language auto-detection: every .c is compiled with $(CC),
# every .cpp with $(CXX); linking uses $(CXX) if at least one
# C++ source exists anywhere, $(CC) otherwise. Mixed C/C++
# programs therefore work out of the box.
#
# Per-program customization goes in that folder's Makefile,
# BEFORE the include line (the ?= assignments below only take
# effect if the variable is still unset):
#
#     LDFLAGS += -lm -lpthread
#     CXXFLAGS = -std=c++20 -O2
#     include ../common.mk
#
# Available targets:
#     all (default)  incremental build + compile_commands.json
#     run            build, then run with: make run ARGS="a b c"
#     test           build and run every test executable
#     clean          remove build/, bin/ and compile_commands.json
#     rebuild        clean + all
# ============================================================

# ==============================
# Toolchain
# ==============================
# CC and CXX are special: make predefines them (CC=cc, CXX=g++),
# so `CC ?= gcc` would never apply — the variable is already set.
# $(origin) tells WHERE a variable got its value: "default" means
# make's builtin, so we replace it; any explicit value (project
# Makefile, command line, environment) is left untouched.
ifeq ($(origin CC),default)
CC 			:= gcc
endif
ifeq ($(origin CXX),default)
CXX 		:= g++
endif

# Compiler flags, overridable per program (?= assigns only if the
# variable is still unset):
#     -std=...                   language standard (C99 / C++23)
#     -g                         debug symbols, needed by CodeLLDB
#     -Wall -Wextra -Wpedantic   wide warning coverage
#     -Werror                    warnings are errors: must compile clean
CFLAGS   	?= -std=c99   -g -Wall -Wextra -Werror -Wpedantic
CXXFLAGS 	?= -std=c++23 -g -Wall -Wextra -Werror -Wpedantic

# Linker flags / libraries; projects typically append with
# `LDFLAGS += -lm` before including this file.
LDFLAGS  	?=

# ==============================
# Layout
# ==============================
SRC_DIR   	?= src
INC_DIR   	?= include
TEST_DIR  	?= test
EXT_DIR   	?= external
BUILD_DIR 	?= build
BIN_DIR   	?= bin
TARGET    	?= $(BIN_DIR)/main

# Every external/<lib>/include found becomes a -I path; a library
# with no include/ folder is simply skipped by $(wildcard).
EXT_INC_DIRS 	:= $(wildcard $(EXT_DIR)/*/include)
EXT_SRC_DIRS 	:= $(wildcard $(EXT_DIR)/*/src)

# Preprocessor flags, shared by C and C++ compiles:
#     -I...          header search paths: include/ plus every
#                    external/<lib>/include (a missing dir is harmless)
#     -MMD           while compiling, also write a .d file listing the
#                    headers the object depends on (project headers
#                    only; -MD would track system headers too)
#     -MP            add a phony target per header, so deleting a
#                    header does not break make with "No rule to
#                    make target ..."
# The .d files make incremental builds header-aware: touch a header
# and every object that includes it gets recompiled.
#
# _POSIX_C_SOURCE: with -std=c99 (strict ISO) glibc hides everything
# beyond plain C99 — pthread_rwlock_*, sigaction, usleep, ... This
# feature-test macro re-enables the POSIX.1-2008 API while keeping
# the language itself strict.
CPPFLAGS 	+= -I$(INC_DIR) $(addprefix -I,$(EXT_INC_DIRS)) -MMD -MP

# ==============================
# Sources
# ==============================
# find (instead of $(wildcard)) descends into subfolders, so
# src/net/tcp.c is picked up too. stderr is silenced for the case
# where the folder does not exist (find would print an error).
SRCS_C   		:= $(shell find $(SRC_DIR) -name '*.c'   2>/dev/null)
SRCS_CXX 		:= $(shell find $(SRC_DIR) -name '*.cpp' 2>/dev/null)

# External libraries are compiled with the same flags as the project.
EXT_SRCS_C 		:= $(if $(EXT_SRC_DIRS),$(shell find $(EXT_SRC_DIRS) -name '*.c'))
EXT_SRCS_CXX 	:= $(if $(EXT_SRC_DIRS),$(shell find $(EXT_SRC_DIRS) -name '*.cpp'))

TEST_SRCS_C 	:= $(shell find $(TEST_DIR) -name '*.c'   2>/dev/null)
TEST_SRCS_CXX 	:= $(shell find $(TEST_DIR) -name '*.cpp' 2>/dev/null)

# Grouped lists used for objects, the linker choice and the
# compilation database.
LIB_SRCS 		:= $(SRCS_C) $(SRCS_CXX) $(EXT_SRCS_C) $(EXT_SRCS_CXX)
TEST_SRCS 		:= $(TEST_SRCS_C) $(TEST_SRCS_CXX)
ALL_SRCS_C 		:= $(SRCS_C) $(EXT_SRCS_C) $(TEST_SRCS_C)
ALL_SRCS_CXX 	:= $(SRCS_CXX) $(EXT_SRCS_CXX) $(TEST_SRCS_CXX)

# Fail fast with a clear message instead of a cryptic linker error.
ifeq ($(strip $(LIB_SRCS)$(TEST_SRCS)),)
$(error No C/C++ sources found in $(SRC_DIR)/ or $(TEST_DIR)/)
endif

# Program vs library detection: grep the src/ sources for a main
# definition. Convention: main must be written as "int main(" (any
# spacing), on one line. Files listed here are excluded from the
# test link so a program with tests does not end up with two mains.
# The pattern lives in its own variable because a literal '(' inside
# $(if ...) would unbalance make's parenthesis parsing.
MAIN_PATTERN 	:= int[[:space:]]+main[[:space:]]*\(
MAIN_SRCS 		:= $(if $(strip $(SRCS_C)$(SRCS_CXX)),$(shell grep -lE '$(MAIN_PATTERN)' $(SRCS_C) $(SRCS_CXX) 2>/dev/null))

# ==============================
# Objects and binaries
# ==============================
# Each source maps to an object that mirrors its full path:
#     src/net/tcp.c              ->  build/src/net/tcp.c.o
#     external/utinc/src/utinc.c ->  build/external/utinc/src/utinc.c.o
#     test/test_core.c           ->  build/test/test_core.c.o
# The original extension is kept in the object name (.c.o / .cpp.o)
# so foo.c and foo.cpp could coexist without colliding on foo.o.
LIB_OBJS 	:= $(addprefix $(BUILD_DIR)/,$(addsuffix .o,$(LIB_SRCS)))
TEST_OBJS 	:= $(addprefix $(BUILD_DIR)/,$(addsuffix .o,$(TEST_SRCS)))
MAIN_OBJS 	:= $(addprefix $(BUILD_DIR)/,$(addsuffix .o,$(MAIN_SRCS)))
DEPS 		:= $(LIB_OBJS:.o=.d) $(TEST_OBJS:.o=.d)

# Library objects without the program entry point: what the test
# executables link against.
LIB_OBJS_NOMAIN := $(filter-out $(MAIN_OBJS),$(LIB_OBJS))

# Each test source becomes its own executable:
#     test/test_core.c  ->  bin/test_core
TEST_BINS_C 	:= $(patsubst $(TEST_DIR)/%.c,$(BIN_DIR)/%,$(TEST_SRCS_C))
TEST_BINS_CXX 	:= $(patsubst $(TEST_DIR)/%.cpp,$(BIN_DIR)/%,$(TEST_SRCS_CXX))
TEST_BINS 		:= $(TEST_BINS_C) $(TEST_BINS_CXX)

# Link with the C++ driver if any C++ source is present (it pulls in
# the C++ runtime, libstdc++); plain C projects use $(CC). $(strip)
# matters: concatenating empty lists leaves spaces, and $(if) treats
# any non-empty string — even blanks — as true.
LD 			:= $(if $(strip $(ALL_SRCS_CXX)),$(CXX),$(CC))

# Compilation database: clangd reads it to learn the real flags.
COMPDB 		:= compile_commands.json

# Running `make` with no arguments builds everything.
.DEFAULT_GOAL := all

# ==============================
# All rules
# ==============================
# .PHONY marks targets that are commands, not files: they run even
# if a file with that name happens to exist. bin/main is requested
# only when a src/ file actually defines main (see MAIN_SRCS).
.PHONY: all
all: $(if $(MAIN_SRCS),$(TARGET)) $(TEST_BINS) $(COMPDB)

# ==============================
# Linker rules
# ==============================
# Automatic variables used in the recipes below:
#     $@   the target being built
#     $<   the first prerequisite (the source file)
#     $^   all prerequisites (every object, for the link)
# mkdir -p $(dir $@) creates the output folder on demand; the
# leading @ silences the echo of the command itself.
$(TARGET): $(LIB_OBJS)
	@mkdir -p $(dir $@)
	$(LD) $^ -o $@ $(LDFLAGS)

# Static pattern rules ("targets: pattern: prerequisites"): for each
# test binary, % captures the name (bin/test_core -> test_core) and
# selects the matching test object; the library objects (minus the
# program main, if any) complete the link.
$(TEST_BINS_C): $(BIN_DIR)/%: $(BUILD_DIR)/$(TEST_DIR)/%.c.o $(LIB_OBJS_NOMAIN)
	@mkdir -p $(dir $@)
	$(LD) $^ -o $@ $(LDFLAGS)

$(TEST_BINS_CXX): $(BIN_DIR)/%: $(BUILD_DIR)/$(TEST_DIR)/%.cpp.o $(LIB_OBJS_NOMAIN)
	@mkdir -p $(dir $@)
	$(LD) $^ -o $@ $(LDFLAGS)

# ==============================
# Compiler rules
# ==============================
# Pattern rules: how to obtain build/<path>.c.o from <path>.c (same
# idea for .cpp). % matches any path, subfolders included, so these
# two rules cover src/, test/ and external/ alike.
$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(BUILD_DIR)/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

# ==============================
# Run rules
# ==============================
# Build if needed, then execute:  make run ARGS="uno 2 tre"
.PHONY: run
run: all
	./$(TARGET) $(ARGS)

# ==============================
# Test rules
# ==============================
# Build and run every test executable; run them all even if one
# fails, then exit non-zero if any did.
.PHONY: test
test: $(TEST_BINS)
	@rc=0; for t in $(TEST_BINS); do ./$$t || rc=1; done; exit $$rc

# ==============================
# Cleanup rules
# ==============================
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR) $(COMPDB)

# ==============================
# Rebuild rules
# ==============================
.PHONY: rebuild
rebuild: clean all

# ==============================
# Clangd rules
# ==============================
# clangd does not read Makefiles: without a compilation database it
# has no -I paths and flags header includes as "file not found" even
# though the build succeeds. This rule writes one JSON entry per
# source (src/, test/ and external/ included) with the exact command
# used to compile it, so clangd checks C files as C99 and C++ files
# as C++23, with the right includes.
#
# Declared .PHONY so it is regenerated on every build: it costs a
# few printf and stays in sync when sources are added or removed.
#
# Inside the recipe, $$ escapes a dollar for the shell ($$f, $$sep,
# $$(pwd)), while single-$ references ($(CC), $(CFLAGS), ...) are
# expanded by make before the shell runs. `sep` emits '[' before the
# first entry and ',' before each following one.
.PHONY: $(COMPDB)
$(COMPDB):
	@{ \
	    sep='['; \
	    for f in $(ALL_SRCS_C); do \
	        printf '%s\n  {"directory": "%s", "file": "%s", "command": "$(CC) $(CFLAGS) $(CPPFLAGS) -c %s"}' "$$sep" "$$(pwd)" "$$f" "$$f"; \
	        sep=','; \
	    done; \
	    for f in $(ALL_SRCS_CXX); do \
	        printf '%s\n  {"directory": "%s", "file": "%s", "command": "$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c %s"}' "$$sep" "$$(pwd)" "$$f" "$$f"; \
	        sep=','; \
	    done; \
	    printf '\n]\n'; \
	} > $(COMPDB)

# ==============================
# Dependency rules
# ==============================
# Pull in the .d files generated by -MMD: they add each object's
# header prerequisites. The leading '-' tells make to ignore missing
# files (first build, or right after a clean).
-include $(DEPS)
