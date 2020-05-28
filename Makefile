# Compiler
CXX = clang++

# Files
SRCS = $(wildcard src/*.cpp)

# Compiler flags
CXXFLAGS = -std=c++17

# Debug flags
DFLAGS = -Wall -Werror -Wextra -Og

# Build flags
BFLAGS = -O3

# Libraries
LDLIBS = -lSDL2

# Executable name
NAME = NicoGB

# Build
build: $(SRCS)
	$(CXX) $(SRCS) $(CXXFLAGS) $(BFLAGS) $(LDLIBS) -o $(NAME)

# Test
test: $(SRCS)
	$(CXX) $(SRCS) $(CXXFLAGS) $(DFLAGS) -DTEST $(LDLIBS) -o $(NAME)
	./${NAME}
