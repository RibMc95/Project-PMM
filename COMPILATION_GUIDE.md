# Compilation Guide for Munch Maze

## Quick Start

To compile and run your Munch Maze game:

```bash
# Navigate to the project directory
cd "/mnt/c/Users/micah/OneDrive/Desktop/DSA Fall 2025/munch maze"

# Compile the project
make

# Run the game
./munch_maze
```

## Makefile Commands

- `make` or `make all` - Compile the game
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
