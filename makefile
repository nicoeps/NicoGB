# Compiler
CC = clang++

# Files
SRCS = $(wildcard src/*.cpp)

# Compiler flags
CFLAGS = -std=c++17 -Wall -Werror -Wextra -O2

# Libraries
LIBS = -lSDL2

# Executable name
NAME = NicoGB

# Build
all: $(SRCS)
	$(CC) $(SRCS) $(CFLAGS) $(LIBS) -o $(NAME) 
