# Compiler
CC = clang++

# Files
SRCS = $(wildcard src/*.cpp)

# Compiler flags
CFLAGS = -std=c++17

# Debug flags
DFLAGS = -Wall -Werror -Wextra -Og

# Build flags
BFLAGS = -O3

# Libraries
LIBS = -lSDL2

# Executable name
NAME = NicoGB

# Build
build: $(SRCS)
	$(CC) $(SRCS) $(CFLAGS) $(BFLAGS) $(LIBS) -o $(NAME) 

# Test
test: $(SRCS)
	$(CC) $(SRCS) $(CFLAGS) $(DFLAGS) -DTEST $(LIBS) -o $(NAME) 
	./${NAME}
