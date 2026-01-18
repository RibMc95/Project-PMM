#ifndef GAMECONFIG_H
#define GAMECONFIG_H

// Game configuration constants - Authentic Pac-Man dimensions
namespace GameConfig
{
    // Authentic Pac-Man grid dimensions
    const int GRID_WIDTH = 28;  // Original Pac-Man: 28 tiles wide
    const int GRID_HEIGHT = 31; // Original Pac-Man: 31 visible tiles high (36 total, but top/bottom cropped)

    // Cell and sprite sizing (scaled up from original 8x8 pixels for better visibility)
    const int CELL_SIZE = 32;   // 4x scale from original 8x8 for excellent visibility
    const int SPRITE_SIZE = 32; // Size to scale sprites to match cell size

    // Window dimensions (authentic Pac-Man resolution scaled 4x)
    const int WINDOW_WIDTH = GRID_WIDTH * CELL_SIZE;   // 28 * 32 = 896 pixels
    const int WINDOW_HEIGHT = GRID_HEIGHT * CELL_SIZE; // 31 * 32 = 992 pixels

    // Sprite scaling factor (to scale from 100x100 to SPRITE_SIZE)
    const float SPRITE_SCALE = static_cast<float>(SPRITE_SIZE) / 100.0f; // 0.32f

    // Alternative scaled-up option for modern displays (uncomment to use)
    /*
    const int CELL_SIZE = 16;    // 2x scale
    const int SPRITE_SIZE = 16;
    const int WINDOW_WIDTH = GRID_WIDTH * CELL_SIZE;   // 28 * 16 = 448 pixels
    const int WINDOW_HEIGHT = GRID_HEIGHT * CELL_SIZE; // 31 * 16 = 496 pixels
    const float SPRITE_SCALE = static_cast<float>(SPRITE_SIZE) / 100.0f; // 0.16f
    */

    // Even larger option for better visibility
    /*
    const int CELL_SIZE = 24;    // 3x scale
    const int SPRITE_SIZE = 24;
    const int WINDOW_WIDTH = GRID_WIDTH * CELL_SIZE;   // 28 * 24 = 672 pixels
    const int WINDOW_HEIGHT = GRID_HEIGHT * CELL_SIZE; // 31 * 24 = 744 pixels
    const float SPRITE_SCALE = static_cast<float>(SPRITE_SIZE) / 100.0f; // 0.24f
    */
}

class PointConfig
{
    public: 
    static constexpr int POINTS_PER_PELLET = 10;
    static constexpr int POINTS_PER_POWER_PELLET = 50;
    static constexpr int First_Frightened_Ghost_Points = 200;
    static constexpr int Second_Frightened_Ghost_Points = 400;
    static constexpr int Third_Frightened_Ghost_Points = 800;
    static constexpr int Fourth_Frightened_Ghost_Points = 1600;
    static constexpr int CHERRY_POINTS = 300;
    static constexpr int STRAWBERRY_POINTS = 500;
    static constexpr int ORANGE_POINTS = 700;
    static constexpr int APPLE_POINTS = 900;
    static constexpr int GRAPEFRUIT_POINTS = 1200;
    static constexpr int PANCAKE_POINTS = 1500;
}

#endif // GAMECONFIG_H