# Makefile for Munch Maze Game
ifeq ($(OS),Windows_NT)
CXX = x86_64-w64-mingw32-g++
EXE = .exe
else
CXX = g++
EXE =
endif

CXXFLAGS = -std=c++17 -Wall -Wextra
LIBS = -lsfml-graphics -lsfml-window -lsfml-system
TARGET = munch_maze$(EXE)
SOURCES = main.cpp


# Default target
all: $(TARGET)


# Build target
$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) $(LIBS) -o $(TARGET)

# Windows static build (portable .exe)
win_static: CXX = x86_64-w64-mingw32-g++
win_static: CXXFLAGS += -static -static-libgcc -static-libstdc++
win_static: LIBS = -lsfml-graphics-s -lsfml-window-s -lsfml-system-s -lopengl32 -lgdi32 -lwinmm -lfreetype -ljpeg -lws2_32
win_static: TARGET = munch_maze.exe
win_static: $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) $(LIBS) -o $(TARGET)


# Clean target
clean:
	rm -f munch_maze munch_maze.exe


# Run target
run: $(TARGET)
	./$(TARGET)


# Debug build
debug: CXXFLAGS += -g -DDEBUG
debug: $(TARGET)

.PHONY: all clean run debug