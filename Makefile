# Makefile for Munch Maze Game (no .exe output)

CXX ?= g++
CXXFLAGS = -std=c++17 -Wall -Wextra
LIBS = -lsfml-graphics -lsfml-window -lsfml-system
TARGET = munch_maze
SOURCES = main.cpp
HEADERS = $(wildcard *.h)

# Default target
all: $(TARGET)

# Build target
$(TARGET): $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SOURCES) $(LIBS) -o $(TARGET)

# Clean target
clean:
	rm -f $(TARGET) munch_maze.exe

# Run target
run: $(TARGET)
	./$(TARGET)

# Debug build
debug: CXXFLAGS += -g -DDEBUG
debug: $(TARGET)

# Portable, statically-linked Windows .exe — no DLLs required. Copy the whole
# folder (munch_maze.exe + Spookies/ muncher/ Objects/ + "practice grid 3.png"
# + Roboto-Black.ttf) onto a USB stick and it runs on any Windows machine.
#
# These link flags are MinGW-specific and may need tuning for your SFML build.
# If the linker reports undefined references like FT_*, png_*, deflate/inflate,
# add the freetype image deps:  -lpng -lz -lbz2  (sometimes also -lharfbuzz).
# Once it works, add  -mwindows -lsfml-main  to hide the console for the demo.
portable:
	$(CXX) $(CXXFLAGS) -DSFML_STATIC $(SOURCES) -o munch_maze.exe \
		-lsfml-graphics-s -lsfml-window-s -lsfml-system-s \
		-lfreetype -lopengl32 -lwinmm -lgdi32 \
		-static -static-libgcc -static-libstdc++

.PHONY: all clean run debug portable