#ifndef GRID_H
#define GRID_H

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <SFML/Graphics.hpp>
#include "GameConfig.h"

// Enum for image tile types
enum class ImageTileType
{
    WALL,        // Blue or Black
    PELLET,      // 
    POWER_PELLET,  // Red
    EMPTY,        // 
    PLAYER_START, // Cyan
    GHOST_SPAWN,  // Green
    SCOREBOARD    // Grey
};

// Helper to map RGB to ImageTileType
static inline ImageTileType getTileTypeFromColor(const sf::Color &color)
{
    // Orange: Pellet (255,126,0) ±40
    if (abs(color.r - 255) < 40 && abs(color.g - 126) < 40 && abs(color.b - 0) < 40)
        return ImageTileType::PELLET;
    // Red: Power pellet (237,28,36) ±40
    if (abs(color.r - 237) < 40 && abs(color.g - 28) < 40 && abs(color.b - 36) < 40)
        return ImageTileType::POWER_PELLET;
    // Grey: Scoreboard (70,70,70) ±20
    if (abs(color.r - 70) < 20 && abs(color.g - 70) < 20 && abs(color.b - 70) < 20)
        return ImageTileType::SCOREBOARD;
    // Cyan: Player start (0,183,239) ±20
    if (abs(color.r - 0) < 20 && abs(color.g - 183) < 20 && abs(color.b - 239) < 20)
        return ImageTileType::PLAYER_START;
    // Green: Ghost spawn (168,230,29) ±10
    if (abs(color.r - 168) < 10 && abs(color.g - 230) < 10 && abs(color.b - 29) < 10)
        return ImageTileType::GHOST_SPAWN;
    // Blue: Wall (0,14,214) ±5 tolerance for PNG/SFML shifts
    if (abs(color.r - 0) <= 5 && abs(color.g - 14) <= 5 && abs(color.b - 214) <= 5)
        return ImageTileType::WALL;
    // Black: Wall (0,0,0)
    if (color.r < 20 && color.g < 20 && color.b < 20)
        return ImageTileType::WALL;
    // Everything else: Empty (not wall)
    return ImageTileType::EMPTY;
}

// Enhanced CellType enum using bit flags
enum CellType
{
    EMPTY = 0,        // 0000
    WALL = 1,         // 0001
    PELLET = 2,       // 0010
    POWER_PELLET = 4, // 0100
    GHOST_SPAWN = 8,  // 1000
    PLAYER_START = 16 // 10000
};

// Operator overloads for bit flag operations
inline CellType operator|(CellType a, CellType b)
{
    return static_cast<CellType>(static_cast<int>(a) | static_cast<int>(b));
}

inline CellType operator&(CellType a, CellType b)
{
    return static_cast<CellType>(static_cast<int>(a) & static_cast<int>(b));
}

inline CellType operator~(CellType a)
{
    return static_cast<CellType>(~static_cast<int>(a));
}

class Grid
{
private:
    int width;
    int height;
    std::vector<std::vector<int>> cells; // Single grid using bit flags

    // Spawn point tracking
    int playerStartX = -1;
    int playerStartY = -1;
    int ghostSpawnX = -1;
    int ghostSpawnY = -1;

public:
    // Constructor
    Grid(int width = 28, int height = 31, bool autoInitialize = true);

    // Bit flag utility methods
    bool hasFlag(int x, int y, CellType flag) const;
    void addFlag(int x, int y, CellType flag);
    void removeFlag(int x, int y, CellType flag);
    void setFlags(int x, int y, int flags);
    int getFlags(int x, int y) const;

    // Wall methods (using bit flags)
    bool isWall(int x, int y) const;
    void setWall(int x, int y, bool isWall);

    // Cell type methods (backward compatibility)
    CellType getCellType(int x, int y) const;
    void setCellType(int x, int y, CellType type);

    // Enhanced pellet methods using bit flags
    bool hasPellet(int x, int y) const;
    bool hasPowerPellet(int x, int y) const;
    void setPellet(int x, int y, bool has);
    void setPowerPellet(int x, int y, bool has);

    // Utility methods
    bool isValidPosition(int x, int y) const;

    // Getters
    int getWidth() const;
    int getHeight() const;

    // Spawn point methods
    bool isPlayerStart(int x, int y) const;
    bool isGhostSpawn(int x, int y) const;
    void setPlayerStart(int x, int y);
    void setGhostSpawn(int x, int y);

    // Spawn point getters
    int getPlayerStartX() const { return playerStartX; }
    int getPlayerStartY() const { return playerStartY; }
    int getGhostSpawnX() const { return ghostSpawnX; }
    int getGhostSpawnY() const { return ghostSpawnY; }

    // Maze initialization
    void initializeMaze();
    void clearMaze();

    // Method to load maze from image
    bool loadMazeFromImage(const std::string &imagePath);
};

// Inline implementations
inline Grid::Grid(int w, int h, bool autoInitialize) : width(w), height(h)
{
    cells.resize(height, std::vector<int>(width, EMPTY));

    if (autoInitialize)
    {
        initializeMaze();
    }
}

// Bit flag utility methods
inline bool Grid::hasFlag(int x, int y, CellType flag) const
{
    if (!isValidPosition(x, y))
        return false;
    return (cells[y][x] & flag) != 0;
}

inline void Grid::addFlag(int x, int y, CellType flag)
{
    if (isValidPosition(x, y))
    {
        cells[y][x] |= flag;

        // Update spawn point tracking
        if (flag == PLAYER_START)
        {
            playerStartX = x;
            playerStartY = y;
        }
        else if (flag == GHOST_SPAWN)
        {
            ghostSpawnX = x;
            ghostSpawnY = y;
        }
    }
}

inline void Grid::removeFlag(int x, int y, CellType flag)
{
    if (isValidPosition(x, y))
    {
        cells[y][x] &= ~flag;

        // Clear spawn point tracking if removing spawn flags
        if (flag == PLAYER_START && playerStartX == x && playerStartY == y)
        {
            playerStartX = -1;
            playerStartY = -1;
        }
        else if (flag == GHOST_SPAWN && ghostSpawnX == x && ghostSpawnY == y)
        {
            ghostSpawnX = -1;
            ghostSpawnY = -1;
        }
    }
}

inline void Grid::setFlags(int x, int y, int flags)
{
    if (isValidPosition(x, y))
    {
        cells[y][x] = flags;
    }
}

inline int Grid::getFlags(int x, int y) const
{
    if (!isValidPosition(x, y))
        return WALL; // Treat out-of-bounds as walls
    return cells[y][x];
}

inline bool Grid::isWall(int x, int y) const
{
    if (!isValidPosition(x, y))
        return true;
    return hasFlag(x, y, WALL);
}

inline void Grid::setWall(int x, int y, bool isWall)
{
    if (isValidPosition(x, y))
    {
        if (isWall)
        {
            addFlag(x, y, WALL);
        }
        else
        {
            removeFlag(x, y, WALL);
        }
    }
}

inline CellType Grid::getCellType(int x, int y) const
{
    if (!isValidPosition(x, y))
        return WALL;

    int flags = cells[y][x];

    // Return the highest priority flag for backward compatibility
    if (flags & WALL)
        return WALL;
    if (flags & POWER_PELLET)
        return POWER_PELLET;
    if (flags & PELLET)
        return PELLET;
    if (flags & GHOST_SPAWN)
        return GHOST_SPAWN;
    if (flags & PLAYER_START)
        return PLAYER_START;

    return EMPTY;
}

inline void Grid::setCellType(int x, int y, CellType type)
{
    if (isValidPosition(x, y))
    {
        // Clear all flags first, then set the specific type
        cells[y][x] = EMPTY;
        if (type != EMPTY)
        {
            addFlag(x, y, type);
        }
    }
}

inline bool Grid::isValidPosition(int x, int y) const
{
    return (x >= 0 && x < width && y >= 0 && y < height);
}

inline int Grid::getWidth() const
{
    return width;
}

inline int Grid::getHeight() const
{
    return height;
}

inline bool Grid::hasPellet(int x, int y) const
{
    if (!isValidPosition(x, y))
        return false;
    return hasFlag(x, y, PELLET);
}

inline bool Grid::hasPowerPellet(int x, int y) const
{
    if (!isValidPosition(x, y))
        return false;
    return hasFlag(x, y, POWER_PELLET);
}

inline void Grid::setPellet(int x, int y, bool has)
{
    if (isValidPosition(x, y))
    {
        if (has)
        {
            addFlag(x, y, PELLET);
        }
        else
        {
            removeFlag(x, y, PELLET);
        }
    }
}

inline void Grid::setPowerPellet(int x, int y, bool has)
{
    if (isValidPosition(x, y))
    {
        if (has)
        {
            addFlag(x, y, POWER_PELLET);
        }
        else
        {
            removeFlag(x, y, POWER_PELLET);
        }
    }
}

// Spawn point methods
inline bool Grid::isPlayerStart(int x, int y) const
{
    return hasFlag(x, y, PLAYER_START);
}

inline bool Grid::isGhostSpawn(int x, int y) const
{
    return hasFlag(x, y, GHOST_SPAWN);
}

inline void Grid::setPlayerStart(int x, int y)
{
    // Remove previous player start if it exists
    if (playerStartX != -1 && playerStartY != -1)
    {
        removeFlag(playerStartX, playerStartY, PLAYER_START);
    }
    addFlag(x, y, PLAYER_START);
}

inline void Grid::setGhostSpawn(int x, int y)
{
    // Remove previous ghost spawn if it exists
    if (ghostSpawnX != -1 && ghostSpawnY != -1)
    {
        removeFlag(ghostSpawnX, ghostSpawnY, GHOST_SPAWN);
    }
    addFlag(x, y, GHOST_SPAWN);
}

inline void Grid::initializeMaze()
{
    // Clear everything first
    clearMaze();

    std::cout << "Creating blank maze - ready to build row by row!" << std::endl;

    // Create completely blank maze - no walls, no pellets, just empty space
    // All cells are already EMPTY from clearMaze()

    // Set basic spawn points in safe locations
    playerStartX = 1;
    playerStartY = 1;
    addFlag(playerStartX, playerStartY, PLAYER_START);

    ghostSpawnX = width - 2;
    ghostSpawnY = height - 2;
    addFlag(ghostSpawnX, ghostSpawnY, GHOST_SPAWN);

    std::cout << "Blank maze created! Grid size: " << width << "x" << height << std::endl;
    std::cout << "Player start: (" << playerStartX << ", " << playerStartY << ")" << std::endl;
    std::cout << "Ghost spawn: (" << ghostSpawnX << ", " << ghostSpawnY << ")" << std::endl;
}

inline bool Grid::loadMazeFromImage(const std::string &filename)
{
    sf::Image image;
    if (!image.loadFromFile(filename))
    {
        std::cout << "Failed to load image: " << filename << std::endl;
        return false;
    }

    sf::Vector2u imageSize = image.getSize();
    std::cout << "Image loaded: " << imageSize.x << "x" << imageSize.y << " pixels" << std::endl;

    // Scale the image to match our grid
    float scaleX = static_cast<float>(imageSize.x) / width;
    float scaleY = static_cast<float>(imageSize.y) / height;

    // Sample the image to create our maze
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            // Sample the center of each cell
            int pixelX = static_cast<int>((x + 0.5f) * scaleX);
            int pixelY = static_cast<int>((y + 0.5f) * scaleY);

            // Clamp to image bounds
            pixelX = std::min(pixelX, static_cast<int>(imageSize.x) - 1);
            pixelY = std::min(pixelY, static_cast<int>(imageSize.y) - 1);

            sf::Color pixelColor = image.getPixel(pixelX, pixelY);

            // Debug first few pixels
            if (x < 5 && y < 5)
            {
                int brightness = (pixelColor.r + pixelColor.g + pixelColor.b) / 3;
                std::cout << "Pixel (" << x << "," << y << ") RGB(" << (int)pixelColor.r
                          << "," << (int)pixelColor.g << "," << (int)pixelColor.b
                          << ") brightness=" << brightness << std::endl;
            }

            // Only treat blue/black as walls for rendering
            if ((pixelColor.r < 40 && pixelColor.g < 40 && pixelColor.b < 40) ||                             // black
                (abs(pixelColor.r - 0) <= 5 && abs(pixelColor.g - 14) <= 5 && abs(pixelColor.b - 214) <= 5)) // blue (±5 tolerance)
            {
                addFlag(x, y, WALL);
            }

            // Use enum map only for logic (object placement)
            ImageTileType tileType = getTileTypeFromColor(pixelColor);
            switch (tileType)
            {
            case ImageTileType::PELLET:
                // Only place pellet if not wall, ghost spawn, or player start
                if (!hasFlag(x, y, WALL) && !hasFlag(x, y, GHOST_SPAWN) && !hasFlag(x, y, PLAYER_START))
                    addFlag(x, y, PELLET);
                break;
            case ImageTileType::POWER_PELLET:
                if (!hasFlag(x, y, WALL) && !hasFlag(x, y, GHOST_SPAWN) && !hasFlag(x, y, PLAYER_START))
                    addFlag(x, y, POWER_PELLET);
                break;
            case ImageTileType::PLAYER_START:
                addFlag(x, y, PLAYER_START);
                break;
            case ImageTileType::GHOST_SPAWN:
                addFlag(x, y, GHOST_SPAWN);
                break;
            case ImageTileType::SCOREBOARD:
                // Nothing, reserved for scoreboard
                break;
            case ImageTileType::WALL:
            case ImageTileType::EMPTY:
            default:
                // No flag for empty or wall (wall already handled above)
                break;
            }
        }
    }

    std::cout << "Maze loaded from image successfully!" << std::endl;
    return true;
}

inline void Grid::clearMaze()
{
    for (int y = 0; y < GameConfig::GRID_HEIGHT; y++)
    {
        for (int x = 0; x < GameConfig::GRID_WIDTH; x++)
        {
            cells[y][x] = EMPTY;
        }
    }

    // Reset spawn positions
    playerStartX = -1;
    playerStartY = -1;
    ghostSpawnX = -1;
    ghostSpawnY = -1;
}

#endif // GRID_H