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
    WALL,         // Blue
    PELLET,       // Orange
    POWER_PELLET, // Red
    EMPTY,        // Brown (walkable floor), or anything far from the legend
    PLAYER_START, // Cyan
    GHOST_SPAWN,  // Green
    GHOST_DOOR,   // Magenta (ghost-only gate)
    SCOREBOARD,   // Grey
    TELEPORT      // Pink (side-to-side tunnel)
};

// The one canonical colour legend for authoring maps. Each map tile is drawn
// in one of these colours; add a row here to add a tile type.
struct MazeLegendEntry
{
    int r, g, b;
    ImageTileType type;
};

static const MazeLegendEntry MAZE_LEGEND[] = {
    {0, 14, 214, ImageTileType::WALL},          // blue
    {255, 126, 0, ImageTileType::PELLET},       // orange
    {70, 70, 70, ImageTileType::SCOREBOARD},    // grey
    {156, 90, 60, ImageTileType::EMPTY},        // brown = walkable floor
    {168, 230, 29, ImageTileType::GHOST_SPAWN}, // yellow-green
    {237, 28, 36, ImageTileType::POWER_PELLET}, // red
    {0, 183, 239, ImageTileType::PLAYER_START}, // cyan
    {255, 0, 255, ImageTileType::GHOST_DOOR},   // magenta
    {255, 163, 177, ImageTileType::TELEPORT},   // pink = side-to-side tunnel
};

// Classify a pixel to the NEAREST legend colour (squared RGB distance). This is
// what makes maps robust: a colour only has to be *close* to a legend entry, not
// an exact match. Anything farther than MAX_DIST from every entry is EMPTY floor.
static inline ImageTileType getTileTypeFromColor(const sf::Color &color)
{
    const int MAX_DIST_SQ = 100 * 100; // ~how far a colour may stray and still match
    int bestDistSq = MAX_DIST_SQ + 1;
    ImageTileType best = ImageTileType::EMPTY;

    for (const MazeLegendEntry &e : MAZE_LEGEND)
    {
        int dr = static_cast<int>(color.r) - e.r;
        int dg = static_cast<int>(color.g) - e.g;
        int db = static_cast<int>(color.b) - e.b;
        int distSq = dr * dr + dg * dg + db * db;
        if (distSq < bestDistSq)
        {
            bestDistSq = distSq;
            best = e.type;
        }
    }
    return best;
}

// Enhanced CellType enum using bit flags
enum CellType
{
    EMPTY = 0,         // 0000
    WALL = 1,          // 0001
    PELLET = 2,        // 0010
    POWER_PELLET = 4,  // 0100
    GHOST_SPAWN = 8,   // 1000
    PLAYER_START = 16, // 10000
    GHOST_DOOR = 32,   // 100000
    SCOREBOARD = 64,   // 1000000
    TELEPORT = 128     // 10000000
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

    // Tunnel/teleport tiles (side-to-side warp). Expected: exactly two.
    std::vector<sf::Vector2i> teleportTiles;

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
    bool isGhostDoor(int x, int y) const;
    bool isScoreboard(int x, int y) const;
    bool isTeleport(int x, int y) const;
    sf::Vector2i getTeleportPartner(int x, int y) const;
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
    if (flags & SCOREBOARD)
        return SCOREBOARD;

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

inline bool Grid::isGhostDoor(int x, int y) const
{
    return hasFlag(x, y, GHOST_DOOR);
}

inline bool Grid::isScoreboard(int x, int y) const
{
    return hasFlag(x, y, SCOREBOARD);
}

inline bool Grid::isTeleport(int x, int y) const
{
    return hasFlag(x, y, TELEPORT);
}

// Return the paired tunnel tile (the other teleport tile). Assumes exactly two.
inline sf::Vector2i Grid::getTeleportPartner(int x, int y) const
{
    for (const auto &t : teleportTiles)
    {
        if (t.x != x || t.y != y)
            return t;
    }
    return sf::Vector2i(-1, -1);
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

            // One legend, one classification. Nearest-colour matching means a
            // map only has to be *close* to the legend colours, not exact.
            switch (getTileTypeFromColor(pixelColor))
            {
            case ImageTileType::WALL:
                addFlag(x, y, WALL);
                break;
            case ImageTileType::PELLET:
                addFlag(x, y, PELLET);
                break;
            case ImageTileType::POWER_PELLET:
                addFlag(x, y, POWER_PELLET);
                break;
            case ImageTileType::PLAYER_START:
                addFlag(x, y, PLAYER_START);
                break;
            case ImageTileType::GHOST_SPAWN:
                addFlag(x, y, GHOST_SPAWN);
                break;
            case ImageTileType::GHOST_DOOR:
                addFlag(x, y, GHOST_DOOR);
                addFlag(x, y, WALL); // door blocks the player; ghosts pass via canMove()
                break;
            case ImageTileType::SCOREBOARD:
                addFlag(x, y, SCOREBOARD);
                break;
            case ImageTileType::TELEPORT:
                addFlag(x, y, TELEPORT);
                teleportTiles.emplace_back(x, y);
                break;
            case ImageTileType::EMPTY:
            default:
                break; // walkable floor — no flag
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
    teleportTiles.clear();
}

#endif // GRID_H