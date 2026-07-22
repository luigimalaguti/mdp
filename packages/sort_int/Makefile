EXEC := sort_int

STD_CXX 	:= c++11
NO_ELISION	:= 1
NO_DEP_COPY := 1

CXXFLAGS ?= -std=$(STD_CXX) -g -Wall -Wextra -Werror -Wpedantic

ifdef NO_ELISION
CXXFLAGS += -fno-elide-constructors
endif

ifdef NO_DEP_COPY
CXXFLAGS += -Wno-deprecated-copy
endif

include ../common.mk
