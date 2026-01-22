# Compilation Guide for Munch Maze

## Quick Start

```bash
cd "/path/to/proto muncher"
make             # builds munch_maze
./munch_maze     # run from the repo root so assets are found (on Windows use ./munch_maze or .\\munch_maze)
```

## Prerequisites

- C++17 compiler, `make`, and SFML 2.6+ development libraries
  - Ubuntu/Debian: `sudo apt-get install libsfml-dev`
  - Fedora: `sudo dnf install SFML-devel`
  - Arch: `sudo pacman -S sfml`
- Keep the asset folders next to the executable: `muncher/`, `Spookies/`, `Objects/`, and the maze PNGs (`practice grid 3.png`, optional `Maze.png`).

## Makefile Targets

- `make` / `make all` – build `munch_maze`
- `make run` – build then run from the repo root
- `make debug` – build with debug symbols
- `make clean` – remove built binaries

## Manual Compilation

```bash
g++ -std=c++17 -Wall -Wextra main.cpp -lsfml-graphics -lsfml-window -lsfml-system -o munch_maze
```

## Runtime Assets & Maze Input

- Run from the project root so relative asset paths resolve.
- Sprites/textures live in `muncher/`, `Spookies/`, and `Objects/`.
- Default maze loads from `practice grid 3.png`; if missing, the code falls back to `Maze.png`, then to the built-in layout.
- Custom maze color key (RGB with small tolerance):
  - Walls: Blue (0,14,214) or black
  - Pellet: Orange (~255,126,0)
  - Power pellet: Red (~237,28,36)
  - Player start: Cyan (~0,183,239)
  - Ghost spawn: Green (~168,230,29)
  - Scoreboard zone: Grey (~70,70,70)

## Features Snapshot

- Authentic Pac-Man sizing: 28x31 grid, 32 px cells, 896x992 window
- Ghost AI with scatter/chase/frightened modes and four personalities
- Fruit cycle spawns every 45s (apple → cherry → strawberry → orange → grapefruit → pancake)
- Pellet and power-pellet collection with frightened ghost handling
- Sprite-based animation for player and ghosts; PNG-driven maze input for fast iteration

## Troubleshooting

- **Undefined references**: SFML dev libs not installed or wrong linkage order; use the Makefile targets to get the right flags.
- **Black screen / no maze**: ensure `practice grid 3.png` (or `Maze.png`) is in the same folder as the executable and uses the color key above.
- **Sprites not showing**: keep the `muncher/`, `Spookies/`, and `Objects/` folders beside the binary.
