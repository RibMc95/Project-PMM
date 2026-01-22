# Munch Maze - Ghost AI System

## Overview

This implementation features an intelligent Ghost AI system based on the classic Pac-Man ghost behaviors, adapted for "Munch Maze" game.

## AI Modes

The ghosts cycle through three main modes:

### 1. **Scatter Mode** (7 seconds)

- Each ghost goes to their assigned corner of the maze
- **JACK (Red)**: Top-left corner (1,1)
- **MIKE (Blue)**: Top-right corner (17,1)  
- **SAM (Purple)**: Bottom-left corner (1,19)
- **WILL (Yellow)**: Bottom-right corner (17,19)

### 2. **Chase Mode** (20 seconds)

- Ghosts actively pursue the Muncher using individual strategies
- Each ghost has a unique personality/targeting system

### 3. **Frightened Mode** (6 seconds)

- Triggered when Muncher eats a power pellet
- Ghosts move randomly and can be "eaten"
- Overrides the normal scatter/chase cycle

## Individual Ghost Personalities

###  JACK (Red Ghost) - "The Chaser"

- **Strategy**: Direct pursuit
- **Target**: Muncher's current position
- **Behavior**: Always heads straight toward the Muncher

###  MIKE (Blue Ghost) - "The Ambusher"  

- **Strategy**: Ambush from ahead
- **Target**: 4 tiles ahead of Muncher's direction
- **Behavior**: Tries to cut off the Muncher's path
- **Note**: Includes the original Pac-Man "up direction bug" for authenticity

###  SAM (Purple Ghost) - "The Fickle"

- **Strategy**: Complex targeting using vector math
- **Target**: Based on both Muncher's position AND Chaser's position
- **Behavior**: Most unpredictable, uses offset calculations
- **Calculation**: Takes a point 2 tiles ahead of Muncher, then doubles the vector from Chaser to that point

###  WILL (Yellow Ghost) - "The Bashful"

- **Strategy**: Alternates between chase and retreat
- **Target**: Muncher's position when far away, corner when close
- **Behavior**: Switches to retreat mode when within 8 tiles of Muncher
- **Characteristic**: Gives players opportunities to exploit predictable switches

## Technical Implementation

### Core Classes

- **`GhostAI`**: Main AI controller that manages all ghost behaviors
- **`AIMode`**: Enum for tracking current game mode
- **`GhostPersonality`**: Enum for individual ghost behavior types

### Key Features

- **Mode Timing**: Automatic cycling between scatter and chase modes
- **Power Pellet Integration**: Frightened mode triggered by pellet consumption  
- **Pathfinding**: Smart direction selection to reach target tiles
- **Anti-Reversal Logic**: Ghosts avoid going backwards unless necessary
- **Distance Calculations**: Euclidean distance for optimal targeting

### Usage

```cpp
// Initialize AI controller
GhostAI ghostAI;

// In game loop
ghostAI.update(ghosts, muncher, grid);

// When power pellet eaten
ghostAI.setFrightened();
```

## Customization Options

You can easily adjust the AI behavior by modifying these constants in `Spookie_Chase.h`:

```cpp
static constexpr float SCATTER_TIME = 7.0f;        // Scatter mode duration
static constexpr float CHASE_TIME = 20.0f;         // Chase mode duration  
static constexpr float FRIGHTENED_TIME = 6.0f;     // Frightened mode duration
static constexpr int AMBUSH_DISTANCE = 4;          // Tiles ahead for ambusher
static constexpr float BASHFUL_RETREAT_DISTANCE = 8.0f; // Bashful retreat threshold
```

## Integration with Your Game

The AI system integrates seamlessly with your existing ghost system:

1. **Include the header**: `#include "Spookie_Chase.h"`
2. **Create AI instance**: `GhostAI ghostAI;`
3. **Update each frame**: `ghostAI.update(ghosts, muncher, grid);`
4. **Handle power pellets**: Call `ghostAI.setFrightened()` when power pellet eaten

## Future Enhancements

Potential improvements you could add:

- **Speed Increases**: Implement "Cruise Elroy" mode for increased difficulty
- **Level Progression**: Shorter mode times and faster speeds as levels increase
- **Custom Personalities**: Create new ghost behaviors for variety
- **Difficulty Scaling**: Adjust AI aggressiveness based on player performance
- **Sound Integration**: Add audio cues for mode changes
- **Visual Indicators**: Display current AI mode on screen

## Debugging

The system includes console output for mode changes:

- Mode transitions are logged to help you understand AI behavior
- Power pellet consumption triggers logged when ghosts become frightened

This creates an engaging, varied AI experience that provides the classic Pac-Man challenge while being adaptable to your custom maze designs!
