#ifndef SPOOKIES_H
#define SPOOKIES_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "Grid.h"
#include "GameConfig.h"

enum class GhostType
{
    JACK, // Magenta ghost
    MIKE, // Blue ghost
    SAM,  // Purple ghost
    WILL  // Yellow ghost
};

enum class GhostDirection
{
    UP,
    DOWN,
    LEFT,
    RIGHT
};

enum class GhostState
{
    NORMAL,
    FRIGHTENED,
    RETURNING
};

class Ghost
{
private:
    sf::Vector2i position;
    sf::Vector2f renderPosition;
    sf::Vector2i spawnPosition; // Original spawn point
    GhostType ghostType;
    GhostDirection direction;
    GhostState state;
    int size;

    // Animation components
    std::vector<sf::Texture> normalTextures[4]; // 4 directions
    std::vector<sf::Texture> frightenedTextures;
    std::vector<sf::Texture> eatenTextures; // Directional textures for eaten/returning state
    sf::Sprite sprite;

    // Animation timing
    sf::Clock animationClock;
    float animationSpeed;
    int currentFrame;

    // Movement
    sf::Vector2f targetPosition;
    sf::Clock movementClock;
    float movementSpeed;
    bool isMoving;

    // Eaten state
    bool isEaten;
    sf::Clock eatenTimer;
    static constexpr float EATEN_DISPLAY_TIME = 0.5f; // Show eaten sprite for 0.5 seconds

public:
    // Set position directly (for teleportation)
    void setPosition(int x, int y)
    {
        position = sf::Vector2i(x, y);
        renderPosition = sf::Vector2f(x * size, y * size);
        targetPosition = renderPosition;
        sprite.setPosition(renderPosition);
        isMoving = false;
    }
    // Constructor
    Ghost(int startX, int startY, GhostType type, int gridSize);

    // Methods
    bool loadTextures();
    void updateAnimation();
    void updateMovement();
    void setState(GhostState newState);
    void setDirection(GhostDirection newDirection);
    bool canMove(const Grid &grid, GhostDirection dir) const;
    void startMovement(const Grid &grid, GhostDirection dir);

    // Getters
    sf::Vector2i getPosition() const { return position; }
    sf::Vector2f getRenderPosition() const { return renderPosition; }
    GhostType getGhostType() const { return ghostType; }
    GhostDirection getDirection() const { return direction; }
    GhostState getState() const { return state; }
    sf::Sprite &getSprite() { return sprite; }
    bool getIsMoving() const { return isMoving; }
    bool getIsEaten() const { return isEaten; }
    void setEaten()
    {
        std::cout << "DEBUG: Ghost::setEaten() called. Setting isEaten=true, state=RETURNING" << std::endl;
        isEaten = true;
        eatenTimer.restart();
        setState(GhostState::RETURNING);
    }
};

// Constructor implementation
inline Ghost::Ghost(int startX, int startY, GhostType type, int gridSize)
    : position(startX, startY), renderPosition(startX * gridSize, startY * gridSize),
      spawnPosition(startX, startY),
      ghostType(type), direction(GhostDirection::RIGHT), state(GhostState::NORMAL),
      size(gridSize), animationSpeed(0.3f), currentFrame(0),
      movementSpeed(0.4f), isMoving(false), isEaten(false)
{
    targetPosition = renderPosition;
    loadTextures();
    setState(GhostState::NORMAL);
}

// Load all ghost textures
inline bool Ghost::loadTextures()
{
    std::string ghostName;
    switch (ghostType)
    {
    case GhostType::JACK:
        ghostName = "Jack";
        break;
    case GhostType::MIKE:
        ghostName = "Mike";
        break;
    case GhostType::SAM:
        ghostName = "Sam";
        break;
    case GhostType::WILL:
        ghostName = "Will";
        break;
    }

    // Load directional textures
    sf::Texture upTexture, downTexture, leftTexture, rightTexture;
    if (!upTexture.loadFromFile("Spookies/" + ghostName + "_Up.png") ||
        !downTexture.loadFromFile("Spookies/" + ghostName + "_Down.png") ||
        !leftTexture.loadFromFile("Spookies/" + ghostName + "_Left.png") ||
        !rightTexture.loadFromFile("Spookies/" + ghostName + "_Right.png"))
    {
        return false;
    }

    normalTextures[0].push_back(upTexture);    // UP
    normalTextures[1].push_back(downTexture);  // DOWN
    normalTextures[2].push_back(leftTexture);  // LEFT
    normalTextures[3].push_back(rightTexture); // RIGHT

    // Load frightened textures
    sf::Texture frightenedTexture1, frightenedTexture2;
    if (!frightenedTexture1.loadFromFile("Spookies/Spookie_Bonus_1.png") ||
        !frightenedTexture2.loadFromFile("Spookies/Spookie_Bonus_2.png"))
    {
        return false;
    }
    frightenedTextures.push_back(frightenedTexture1);
    frightenedTextures.push_back(frightenedTexture2);

    // Load eaten texture
    sf::Texture eatenUpTexture, eatenDownTexture, eatenLeftTexture, eatenRightTexture;
    if (!eatenUpTexture.loadFromFile("Spookies/Spookie_Eaten_Up.png") ||
        !eatenDownTexture.loadFromFile("Spookies/Spookie_Eaten_Down.png") ||
        !eatenLeftTexture.loadFromFile("Spookies/Spookie_Eaten_Left.png") ||
        !eatenRightTexture.loadFromFile("Spookies/Spookie_Eaten_Right.png"))
    {
        return false;
    }

    eatenTextures.push_back(eatenUpTexture);
    eatenTextures.push_back(eatenDownTexture);
    eatenTextures.push_back(eatenLeftTexture);
    eatenTextures.push_back(eatenRightTexture);

    return true;
}

// Update animation frames
inline void Ghost::updateAnimation()
{
    if (animationClock.getElapsedTime().asSeconds() >= animationSpeed)
    {
        if (state == GhostState::FRIGHTENED)
        {
            currentFrame = (currentFrame + 1) % frightenedTextures.size();
            sprite.setTexture(frightenedTextures[currentFrame]);
        }
        else if (state == GhostState::RETURNING)
        {
            int dirIndex = static_cast<int>(direction);
            if (dirIndex >= 0 && dirIndex < static_cast<int>(eatenTextures.size()))
            {
                sprite.setTexture(eatenTextures[dirIndex]);
            }
        }
        else
        {
            // Use normal directional texture
            int dirIndex = static_cast<int>(direction);
            if (!normalTextures[dirIndex].empty())
            {
                sprite.setTexture(normalTextures[dirIndex][0]);
            }
        }

        // Scale the sprite from 100x100 to appropriate size
        sprite.setScale(GameConfig::SPRITE_SCALE, GameConfig::SPRITE_SCALE);
        animationClock.restart();
    }
}

// Update smooth movement between grid positions
inline void Ghost::updateMovement()
{
    if (isMoving)
    {
        float elapsed = movementClock.getElapsedTime().asSeconds();
        float progress = elapsed / movementSpeed;

        if (progress >= 1.0f)
        {
            // Movement complete
            renderPosition = targetPosition;
            position = sf::Vector2i(targetPosition.x / size, targetPosition.y / size);
            isMoving = false;

            // If eaten and reached spawn, reset to normal
            if (isEaten && position == spawnPosition)
            {
                std::cout << "Ghost reached spawn at (" << position.x << "," << position.y << "). Resetting to NORMAL." << std::endl;
                isEaten = false;
                setState(GhostState::NORMAL);
            }
        }
        else
        {
            // Interpolate between start and target positions
            sf::Vector2f startPos(position.x * size, position.y * size);
            renderPosition.x = startPos.x + (targetPosition.x - startPos.x) * progress;
            renderPosition.y = startPos.y + (targetPosition.y - startPos.y) * progress;
        }

        sprite.setPosition(renderPosition);
    }
    else if (isEaten)
    {
        // If eaten and not moving, start moving toward spawn
        if (position != spawnPosition)
        {
            std::cout << "Ghost moving from (" << position.x << "," << position.y << ") to spawn (" << spawnPosition.x << "," << spawnPosition.y << ")" << std::endl;
            // Simple pathfinding: move closer to spawn
            int nextX = position.x;
            int nextY = position.y;

            if (position.x < spawnPosition.x)
                nextX++;
            else if (position.x > spawnPosition.x)
                nextX--;
            else if (position.y < spawnPosition.y)
                nextY++;
            else if (position.y > spawnPosition.y)
                nextY--;

            sf::Vector2i nextPos(nextX, nextY);
            targetPosition = sf::Vector2f(nextPos.x * size, nextPos.y * size);
            isMoving = true;
            movementClock.restart();

            // Update direction based on movement
            if (nextX > position.x)
                setDirection(GhostDirection::RIGHT);
            else if (nextX < position.x)
                setDirection(GhostDirection::LEFT);
            else if (nextY > position.y)
                setDirection(GhostDirection::DOWN);
            else if (nextY < position.y)
                setDirection(GhostDirection::UP);
        }
    }
}

// Set ghost state
inline void Ghost::setState(GhostState newState)
{
    if (state != newState)
    {
        state = newState;
        currentFrame = 0;
        animationClock.restart();

        // Update texture based on new state
        if (state == GhostState::FRIGHTENED && !frightenedTextures.empty())
        {
            sprite.setTexture(frightenedTextures[0]);
        }
        else if (state == GhostState::RETURNING)
        {
            std::cout << "DEBUG: Ghost setState RETURNING. eatenTextures.size()=" << eatenTextures.size() << ", direction=" << static_cast<int>(direction) << std::endl;
            int dirIndex = static_cast<int>(direction);
            if (dirIndex >= 0 && dirIndex < static_cast<int>(eatenTextures.size()))
            {
                std::cout << "DEBUG: Setting eaten texture at index " << dirIndex << std::endl;
                sprite.setTexture(eatenTextures[dirIndex]);
            }
            else
            {
                std::cout << "DEBUG: WARNING - dirIndex " << dirIndex << " out of range for eatenTextures!" << std::endl;
            }
        }
        else
        {
            int dirIndex = static_cast<int>(direction);
            if (!normalTextures[dirIndex].empty())
            {
                sprite.setTexture(normalTextures[dirIndex][0]);
            }
        }

        sprite.setScale(GameConfig::SPRITE_SCALE, GameConfig::SPRITE_SCALE);
    }
}

// Set movement direction
inline void Ghost::setDirection(GhostDirection newDirection)
{
    direction = newDirection;
    // Update texture when direction changes
    int dirIndex = static_cast<int>(direction);

    if (state == GhostState::FRIGHTENED)
    {
        return;
    }

    if (state == GhostState::RETURNING)
    {
        if (dirIndex >= 0 && dirIndex < static_cast<int>(eatenTextures.size()))
        {
            sprite.setTexture(eatenTextures[dirIndex]);
        }
    }
    else if (!normalTextures[dirIndex].empty())
    {
        sprite.setTexture(normalTextures[dirIndex][0]);
    }

    sprite.setScale(GameConfig::SPRITE_SCALE, GameConfig::SPRITE_SCALE);
}

// Check if ghost can move in a direction
inline bool Ghost::canMove(const Grid &grid, GhostDirection dir) const
{
    sf::Vector2i newPos = position;

    switch (dir)
    {
    case GhostDirection::UP:
        newPos.y--;
        break;
    case GhostDirection::DOWN:
        newPos.y++;
        break;
    case GhostDirection::LEFT:
        newPos.x--;
        break;
    case GhostDirection::RIGHT:
        newPos.x++;
        break;
    }

    // Check bounds
    if (newPos.x < 0 || newPos.x >= grid.getWidth() ||
        newPos.y < 0 || newPos.y >= grid.getHeight())
    {
        return false;
    }

    // Check for walls
    return !grid.isWall(newPos.x, newPos.y);
}

// Start movement animation in a direction
inline void Ghost::startMovement(const Grid &grid, GhostDirection dir)
{
    if (isMoving)
    {
        return; // Already moving
    }

    if (!canMove(grid, dir))
    {
        return; // Can't move in that direction
    }

    setDirection(dir);
    sf::Vector2i newGridPos = position;

    switch (dir)
    {
    case GhostDirection::UP:
        newGridPos.y--;
        break;
    case GhostDirection::DOWN:
        newGridPos.y++;
        break;
    case GhostDirection::LEFT:
        newGridPos.x--;
        break;
    case GhostDirection::RIGHT:
        newGridPos.x++;
        break;
    }

    targetPosition = sf::Vector2f(newGridPos.x * size, newGridPos.y * size);
    isMoving = true;
    movementClock.restart();
}

#endif // SPOOKIES_H