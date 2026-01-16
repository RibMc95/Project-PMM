#ifndef MAZE_H
#define MAZE_H

#include <vector>
#include <string>
#include <iostream>
#include <SFML/Graphics.hpp>
#include "GameConfig.h"

// Simple maze structure that focuses only on static layout
class Maze
{
private:
    std::vector<std::vector<bool>> walls;
    int width = GameConfig::GRID_WIDTH;
    int height = GameConfig::GRID_HEIGHT;

    // Spawn points
    int playerStartX = 14;
    int playerStartY = 23;
    int ghostSpawnX = 14;
    int ghostSpawnY = 15;

public:
    // Constructor
    Maze();

    // Load maze from image
    bool loadFromImage(const std::string &filename);

    // Load fallback maze
    void loadFallback();

    // Find suitable spawn points in corridor areas
    void findSpawnPoints();

    // Wall checking
    bool isWall(int x, int y) const;
    bool isValidPosition(int x, int y) const;

    // Getters
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getPlayerStartX() const { return playerStartX; }
    int getPlayerStartY() const { return playerStartY; }
    int getGhostSpawnX() const { return ghostSpawnX; }
    int getGhostSpawnY() const { return ghostSpawnY; }

    // Validation methods
    bool validateGrid() const;
    void printGridSample(int startX = 0, int startY = 0, int sizeX = 10, int sizeY = 10) const;
};

// Implementation
inline Maze::Maze()
{
    walls.resize(height, std::vector<bool>(width, false));

    // Try to load from image first
    if (!loadFromImage("Maze.png"))
    {
        std::cout << "Could not load Maze.png, using fallback layout..." << std::endl;
        loadFallback();
    }
}

inline bool Maze::loadFromImage(const std::string &filename)
{
    sf::Image image;
    if (!image.loadFromFile(filename))
    {
        return false;
    }

    sf::Vector2u imageSize = image.getSize();
    std::cout << "Image loaded: " << imageSize.x << "x" << imageSize.y << " pixels" << std::endl;

    // Scale the image to match our grid
    float scaleX = static_cast<float>(imageSize.x) / width;
    float scaleY = static_cast<float>(imageSize.y) / height;

    std::cout << "Scale factors: " << scaleX << "x" << scaleY << std::endl;

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

            // Enhanced color analysis for better tile type detection
            int brightness = (pixelColor.r + pixelColor.g + pixelColor.b) / 3;

            // Debug output for spawn points and some sample pixels
            if ((x < 5 && y < 5) || (x == 14 && (y == 15 || y == 23)))
            {
                std::cout << "Pixel (" << x << "," << y << ") RGB("
                          << (int)pixelColor.r << "," << (int)pixelColor.g << "," << (int)pixelColor.b
                          << ") brightness=" << brightness << std::endl;
            }

            // Color-based tile type detection
            if (brightness < 50)
            {
                // Very dark pixels = walls
                walls[y][x] = true;
            }
            else if (pixelColor.r > 200 && pixelColor.g > 200 && pixelColor.b < 100)
            {
                // Yellow-ish pixels = special areas (could be power pellet spots)
                walls[y][x] = false;
                std::cout << "Found yellow area at (" << x << "," << y << ")" << std::endl;
            }
            else if (pixelColor.r < 100 && pixelColor.g < 100 && pixelColor.b > 200)
            {
                // Blue-ish pixels = corridors
                walls[y][x] = false;
            }
            else if (brightness > 200)
            {
                // Very bright pixels = special corridors or spawn areas
                walls[y][x] = false;
            }
            else
            {
                // Medium brightness = regular corridor
                walls[y][x] = false;
            }
        }
    }

    std::cout << "Maze loaded from " << filename << " successfully!" << std::endl;

    // Find suitable spawn points in corridor areas
    findSpawnPoints();

    // Debug spawn point detection
    std::cout << "Checking spawn points:" << std::endl;
    std::cout << "  Player spawn (" << playerStartX << "," << playerStartY << ") is "
              << (isWall(playerStartX, playerStartY) ? "WALL" : "CORRIDOR") << std::endl;
    std::cout << "  Ghost spawn (" << ghostSpawnX << "," << ghostSpawnY << ") is "
              << (isWall(ghostSpawnX, ghostSpawnY) ? "WALL" : "CORRIDOR") << std::endl; // Debug: Count wall vs corridor ratio
    int wallCount = 0, corridorCount = 0;
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            if (walls[y][x])
                wallCount++;
            else
                corridorCount++;
        }
    }
    std::cout << "Maze analysis: " << wallCount << " walls, " << corridorCount << " corridors" << std::endl;

    return true;
}

inline void Maze::loadFallback()
{
    // Simple fallback maze - classic Pac-Man inspired
    std::vector<std::string> layout = {
        "WWWWWWWWWWWWWWWWWWWWWWWWWWWW",
        "WEEEEEEEEEEEWWWEEEEEEEEEEW",
        "WEWWWWEWWWWEWWWEWWWWEWWWWEW",
        "WPEEEEEEEEEEEEEEEEEEEEEEEPW",
        "WEWWWWEWEWWWWWWWWEWEWWWWEW",
        "WEEEEEWEWEEEEWEEEEWEWEEEEW",
        "WWWWWEWEWEEEEWEEEEWEWWWWWW",
        "EEEEEWEWWWWWWWWWWWWEWEWEEEEE",
        "WWWWWEWEWWWWWWWWWWEWEWWWWW",
        "WEEEEEWEWEEEEWEEEEWEWEEEEW",
        "WEWWWWEWWWWEWEEWEWWWEWWWWEW",
        "WEEEEEEEEEEEEEEEEEEEEEEEEW",
        "WEWWWWEWEWWWWWWWWEWEWWWWEW",
        "WEEEEEWEWEEEEWEEEEWEWEEEEW",
        "WWWWWEWEWEEEEWEEEEWEWWWWWW",
        "WEEEEWEEEEEEGGGGEEEEEWEEEW",
        "WEWWWWEWWWWWGGGGWWWWEWWWWEW",
        "WEEEEEEEEEEEGGGGEEEEEEEEEEW",
        "WWWWWEWEWWWWGGGGWWWWEWEWWWW",
        "WEEEEEWEWEEEEGEEEEWEWEEEEW",
        "WEWWWWEWEWWWWWWWWEWEWWWWEW",
        "WEEEEEEEEEEEWEEWEEEEEEEEEW",
        "WWWWEWWWWEEEWEEWEEWEWWWWWW",
        "WEEEEEEEEEEESEEEEEEEEEEEW",
        "WEWWWWEWEWWWWWWWWEWEWWWWEW",
        "WEEEEEWEWEEEEWEEEEWEWEEEEW",
        "WWWWWEWEWEEEEWEEEEWEWWWWWW",
        "WEEEEEEEEEEEEEEEEEEEEEEEW",
        "WEWWWWEWWWWWEEWEWWWWEWWWWEW",
        "WPEEEEEEEEEEWEEWEEEEEEEEEPW",
        "WWWWWWWWWWWWWWWWWWWWWWWWWWWW"};

    for (int y = 0; y < height && y < static_cast<int>(layout.size()); y++)
    {
        for (int x = 0; x < width && x < static_cast<int>(layout[y].length()); x++)
        {
            char cell = layout[y][x];

            switch (cell)
            {
            case 'W':
                walls[y][x] = true;
                break;
            case 'S':
                walls[y][x] = false;
                // We'll override playerStartX/playerStartY below
                break;
            case 'G':
                walls[y][x] = false;
                if (x == 14 && y == 16)
                {
                    ghostSpawnX = x;
                    ghostSpawnY = y;
                }
                break;
            default:
                walls[y][x] = false;
                break;
            }
        }
    }
    // Set player start to cyan tile (14, 19)
    playerStartX = 14;
    playerStartY = 19;
}

inline bool Maze::isWall(int x, int y) const
{
    if (!isValidPosition(x, y))
        return true;
    return walls[y][x];
}

inline bool Maze::isValidPosition(int x, int y) const
{
    return (x >= 0 && x < width && y >= 0 && y < height);
}

inline bool Maze::validateGrid() const
{
    // Check if grid has reasonable wall/corridor ratio
    int wallCount = 0, corridorCount = 0;
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            if (walls[y][x])
                wallCount++;
            else
                corridorCount++;
        }
    }

    float wallRatio = static_cast<float>(wallCount) / (wallCount + corridorCount);
    std::cout << "Grid validation - Wall ratio: " << (wallRatio * 100) << "%" << std::endl;

    // Check if spawn points are valid
    bool playerSpawnValid = !isWall(playerStartX, playerStartY);
    bool ghostSpawnValid = !isWall(ghostSpawnX, ghostSpawnY);

    std::cout << "Player spawn valid: " << (playerSpawnValid ? "YES" : "NO") << std::endl;
    std::cout << "Ghost spawn valid: " << (ghostSpawnValid ? "YES" : "NO") << std::endl;

    return playerSpawnValid && ghostSpawnValid && wallRatio > 0.2f && wallRatio < 0.95f;
}

inline void Maze::printGridSample(int startX, int startY, int sizeX, int sizeY) const
{
    std::cout << "\nMaze Grid Sample (" << startX << "," << startY << ") to ("
              << (startX + sizeX - 1) << "," << (startY + sizeY - 1) << "):" << std::endl;

    for (int y = startY; y < startY + sizeY && y < height; y++)
    {
        for (int x = startX; x < startX + sizeX && x < width; x++)
        {
            if (x == playerStartX && y == playerStartY)
            {
                std::cout << "P"; // Player start
            }
            else if (x == ghostSpawnX && y == ghostSpawnY)
            {
                std::cout << "G"; // Ghost spawn
            }
            else if (walls[y][x])
            {
                std::cout << "#"; // Wall
            }
            else
            {
                std::cout << "."; // Corridor
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

inline void Maze::findSpawnPoints()
{
    // Find a suitable player spawn point (prefer bottom half of maze)
    bool foundPlayer = false;
    for (int y = height * 3 / 4; y < height && !foundPlayer; y++)
    {
        for (int x = width / 4; x < width * 3 / 4 && !foundPlayer; x++)
        {
            if (!isWall(x, y))
            {
                playerStartX = x;
                playerStartY = y;
                foundPlayer = true;
                std::cout << "Found player spawn at (" << x << "," << y << ")" << std::endl;
            }
        }
    }

    // Find a suitable ghost spawn point (prefer middle of maze)
    bool foundGhost = false;
    for (int y = height / 3; y < height * 2 / 3 && !foundGhost; y++)
    {
        for (int x = width / 4; x < width * 3 / 4 && !foundGhost; x++)
        {
            if (!isWall(x, y) && (x != playerStartX || y != playerStartY))
            {
                ghostSpawnX = x;
                ghostSpawnY = y;
                foundGhost = true;
                std::cout << "Found ghost spawn at (" << x << "," << y << ")" << std::endl;
            }
        }
    }

    // Fallback: if no suitable points found, force some to be corridors
    if (!foundPlayer)
    {
        playerStartX = 3;
        playerStartY = height - 5;
        walls[playerStartY][playerStartX] = false; // Force corridor
        std::cout << "Forced player spawn at (" << playerStartX << "," << playerStartY << ")" << std::endl;
    }

    if (!foundGhost)
    {
        ghostSpawnX = width / 2;
        ghostSpawnY = height / 2;
        walls[ghostSpawnY][ghostSpawnX] = false; // Force corridor
        std::cout << "Forced ghost spawn at (" << ghostSpawnX << "," << ghostSpawnY << ")" << std::endl;
    }
}

#endif // MAZE_H