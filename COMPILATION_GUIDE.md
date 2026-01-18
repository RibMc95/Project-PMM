# Compilation Guide for Munch Maze

## Quick Start

To compile and run your Munch Maze game:

```bash
# Navigate to the project directory
cd "/mnt/c/Users/micah/OneDrive/Desktop/proto muncher"

# Compile the project (Linux/macOS)
make

# Run the game
./munch_maze
```

## Building a Fully Portable Windows .exe

To create a standalone Windows executable that runs on any Windows machine (no command line needed):

1. Install [MinGW-w64](https://www.mingw-w64.org/) and static SFML libraries for Windows.
2. Open a terminal in your project directory.
3. Run:

```bash
make win_static
```

This will produce `munch_maze.exe` which is fully portable. You can double-click it to run on any Windows PC (no extra DLLs required).

**Note:**

- You must have static versions of SFML libraries (e.g., `sfml-graphics-s.lib`, etc.) in your MinGW environment.
- If you see missing DLL errors, ensure you are linking static libraries and have all dependencies available.

## Makefile Commands

- `make` or `make all` - Compile the game (Linux/macOS)
- `make win_static` - Build a fully portable Windows .exe
- `make clean` - Remove compiled files
- `make run` - Compile and run the game
- `make debug` - Compile with debug symbols

## Manual Compilation (if needed)

If you prefer to compile manually without the Makefile:

```bash
g++ -std=c++17 -Wall -Wextra main.cpp -lsfml-graphics -lsfml-window -lsfml-system -o munch_maze
```

## Required Libraries

Make sure you have SFML installed on your system:

- **Ubuntu/Debian**: `sudo apt-get install libsfml-dev`
- **Fedora**: `sudo dnf install SFML-devel`
- **Arch Linux**: `sudo pacman -S sfml`

## Troubleshooting

### Linker Errors

If you get "undefined reference" errors, it usually means:

1. SFML is not installed
2. SFML libraries are not being linked properly
3. Wrong library path

### Solution

The Makefile includes the proper SFML linking flags:

- `-lsfml-graphics` (for graphics/rendering)
- `-lsfml-window` (for window management)  
- `-lsfml-system` (for system utilities)

### Runtime Errors

If the program compiles but crashes at runtime:

1. Make sure you have the required sprite/texture files
2. Check that file paths in the code match your actual file structure
3. Ensure you have proper graphics drivers

## Project Structure

```
munch maze/
├── main.cpp              # Main game loop
├── Spookie_Chase.h       # Ghost AI system
├── Spookies.h           # Ghost class definitions
├── Muncher.h            # Player character
├── Pellet.h             # Pellet/food items
├── Grid.h               # Game grid/maze
├── GameConfig.h         # Configuration constants
├── Makefile             # Build configuration
└── AI_README.md         # AI system documentation
```

## Features Implemented

✅ **Authentic Pac-Man Dimensions**

- 28x31 grid (authentic Pac-Man maze size)
- 24x24 pixel cells (3x scale from original 8x8)
- 672x744 pixel window (perfect for modern displays)
- Classic Pac-Man proportions and layout

✅ **Complete Ghost AI System**

- Scatter/Chase/Frightened modes
- Individual ghost personalities
- Smart pathfinding
- Power pellet interactions

✅ **Game Mechanics**

- Player movement
- Pellet collection
- Power pellet special effects
- Collision detection

✅ **Graphics**

- SFML-based rendering
- Sprite animations
- Smooth movement

## Success

Your Munch Maze game with intelligent Ghost AI is now ready to play! 🎮
