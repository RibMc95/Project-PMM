#ifndef MUNCHER_H
#define MUNCHER_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "Grid.h"
#include "GameConfig.h"

enum class MuncherDirection // Direction enum for Muncher
{
    UP,
    DOWN,
    LEFT,
    RIGHT
};

enum class MuncherState // State enum for animations
{
    IDLE,
    MOVING,
    DYING
};

class Muncher
{
private:
    sf::Vector2i position;       // Muncher's position on the grid
    sf::Vector2f renderPosition; // Smooth rendering position for animation
    MuncherDirection direction;  // Current direction
    MuncherState state;          // Current animation state
    int size;                    // Size of the muncher (grid cell size)

    // Animation components
    std::vector<sf::Texture> idleTextures;
    std::vector<sf::Texture> movingTextures;
    std::vector<sf::Texture> deathTextures;
    sf::Sprite sprite;

    // Animation timing
    sf::Clock animationClock;
    float animationSpeed; // Time between frames in seconds
    int currentFrame;

    // Movement animation
    sf::Vector2f targetPosition;
    sf::Clock movementClock;
    float movementSpeed; // Time to complete one grid movement
    bool isMoving;

public:
    // Constructor
    Muncher(int startX, int startY, int gridSize);

    // Animation methods
    bool loadTextures();
    void updateAnimation();
    void updateMovement();
    void setState(MuncherState newState);

    // Movement methods
    void setDirection(MuncherDirection newDirection);
    bool canMove(const Grid &grid, MuncherDirection dir) const;
    void startMovement(const Grid &grid, MuncherDirection dir); // Getters
    sf::Vector2i getPosition() const
    {
        return position;
    }

    sf::Vector2f getRenderPosition() const
    {
        return renderPosition;
    }
    MuncherDirection getDirection() const
    {
        return direction;
    }
    MuncherState getState() const
    {
        return state;
    }
    sf::Sprite &getSprite()
    {
        return sprite;
    }
    bool getIsMoving() const
    {
        return isMoving;
    }
};

// Constructor implementation
inline Muncher::Muncher(int startX, int startY, int gridSize)
    : position(startX, startY), renderPosition(startX * gridSize, startY * gridSize),
      direction(MuncherDirection::RIGHT), state(MuncherState::IDLE),
      size(gridSize), animationSpeed(0.2f), currentFrame(0),
      movementSpeed(0.3f), isMoving(false)
{
    targetPosition = renderPosition;
    loadTextures();
    setState(MuncherState::IDLE);
    // Set initial sprite position with origin offset
    sprite.setPosition(renderPosition.x + gridSize / 2.0f, renderPosition.y + gridSize / 2.0f);
}

// Load all PNG textures
inline bool Muncher::loadTextures()
{
    // Load idle texture
    sf::Texture idleTexture;
    if (!idleTexture.loadFromFile("muncher/Munch_idle.png"))
    {
        return false;
    }
    idleTextures.push_back(idleTexture);

    // Load moving textures
    sf::Texture moveTexture1, moveTexture2;
    if (!moveTexture1.loadFromFile("muncher/Munch_1.png") || !moveTexture2.loadFromFile("muncher/Munch_2.png"))
    {
        return false;
    }
    movingTextures.push_back(moveTexture1);
    movingTextures.push_back(moveTexture2);

    // Load death textures
    sf::Texture deathTexture1, deathTexture2, deathTexture3, deathTextureFinal;
    if (!deathTexture1.loadFromFile("muncher/Muncher_death_1.png") ||
        !deathTexture2.loadFromFile("muncher/Muncher_death_2.png") ||
        !deathTexture3.loadFromFile("muncher/Muncher_death_3.png") ||
        !deathTextureFinal.loadFromFile("muncher/Muncher_death_final.png"))
    {
        return false;
    }
    deathTextures.push_back(deathTexture1);
    deathTextures.push_back(deathTexture2);
    deathTextures.push_back(deathTexture3);
    deathTextures.push_back(deathTextureFinal);

    return true;
}

// Update animation frames
inline void Muncher::updateAnimation()
{
    if (animationClock.getElapsedTime().asSeconds() >= animationSpeed)
    {
        std::vector<sf::Texture> *currentTextures = nullptr;

        switch (state)
        {
        case MuncherState::IDLE:
            currentTextures = &idleTextures;
            break;
        case MuncherState::MOVING:
            currentTextures = &movingTextures;
            break;
        case MuncherState::DYING:
            currentTextures = &deathTextures;
            break;
        }

        if (currentTextures && !currentTextures->empty())
        {
            currentFrame = (currentFrame + 1) % currentTextures->size();
            sprite.setTexture((*currentTextures)[currentFrame]);
            sprite.setScale(GameConfig::SPRITE_SCALE, GameConfig::SPRITE_SCALE);

            // Set rotation based on direction
            sprite.setOrigin(50.0f, 50.0f); // Set origin to center of 100x100 sprite
            switch (direction)
            {
            case MuncherDirection::RIGHT:
                sprite.setRotation(0.0f);
                break;
            case MuncherDirection::DOWN:
                sprite.setRotation(90.0f);
                break;
            case MuncherDirection::LEFT:
                sprite.setRotation(180.0f);
                break;
            case MuncherDirection::UP:
                sprite.setRotation(270.0f);
                break;
            }
        }

        animationClock.restart();
    }
}

// Update smooth movement between grid positions
inline void Muncher::updateMovement()
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
            setState(MuncherState::IDLE);
        }
        else
        {
            // Interpolate between start and target positions
            sf::Vector2f startPos(position.x * size, position.y * size);
            renderPosition.x = startPos.x + (targetPosition.x - startPos.x) * progress;
            renderPosition.y = startPos.y + (targetPosition.y - startPos.y) * progress;
        }

        // Adjust position for rotated sprite (add half cell size to account for origin at center)
        sprite.setPosition(renderPosition.x + size / 2.0f, renderPosition.y + size / 2.0f);
    }
}

// Set animation state
inline void Muncher::setState(MuncherState newState)
{
    if (state != newState)
    {
        state = newState;
        currentFrame = 0;
        animationClock.restart();

        // Set initial texture for the new state
        std::vector<sf::Texture> *currentTextures = nullptr;
        switch (state)
        {
        case MuncherState::IDLE:
            currentTextures = &idleTextures;
            break;
        case MuncherState::MOVING:
            currentTextures = &movingTextures;
            break;
        case MuncherState::DYING:
            currentTextures = &deathTextures;
            break;
        }

        if (currentTextures && !currentTextures->empty())
        {
            sprite.setTexture((*currentTextures)[0]);
            sprite.setScale(GameConfig::SPRITE_SCALE, GameConfig::SPRITE_SCALE);

            // Set rotation based on direction
            sprite.setOrigin(50.0f, 50.0f); // Set origin to center of 100x100 sprite
            switch (direction)
            {
            case MuncherDirection::RIGHT:
                sprite.setRotation(0.0f);
                break;
            case MuncherDirection::DOWN:
                sprite.setRotation(90.0f);
                break;
            case MuncherDirection::LEFT:
                sprite.setRotation(180.0f);
                break;
            case MuncherDirection::UP:
                sprite.setRotation(270.0f);
                break;
            }
        }
    }
}

// Set movement direction
inline void Muncher::setDirection(MuncherDirection newDirection)
{
    direction = newDirection;
}

// Check if muncher can move in a direction
inline bool Muncher::canMove(const Grid &grid, MuncherDirection dir) const
{
    sf::Vector2i newPos = position;

    switch (dir)
    {
    case MuncherDirection::UP:
        newPos.y--;
        break;
    case MuncherDirection::DOWN:
        newPos.y++;
        break;
    case MuncherDirection::LEFT:
        newPos.x--;
        break;
    case MuncherDirection::RIGHT:
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
inline void Muncher::startMovement(const Grid &grid, MuncherDirection dir)
{
    if (isMoving)
    {
        return; // Already moving
    }

    if (!canMove(grid, dir))
    {
        return; // Can't move in that direction
    }

    direction = dir;
    sf::Vector2i newGridPos = position;

    switch (dir)
    {
    case MuncherDirection::UP:
        newGridPos.y--;
        break;
    case MuncherDirection::DOWN:
        newGridPos.y++;
        break;
    case MuncherDirection::LEFT:
        newGridPos.x--;
        break;
    case MuncherDirection::RIGHT:
        newGridPos.x++;
        break;
    }

    targetPosition = sf::Vector2f(newGridPos.x * size, newGridPos.y * size);
    isMoving = true;
    setState(MuncherState::MOVING);
    movementClock.restart();
}

// Main move function that handles input and updates
void move(Muncher &muncher, const Grid &grid)
{
    // Update animations and movement
    muncher.updateAnimation();
    muncher.updateMovement();

    // Handle input for movement (you can modify this based on your input system)
    if (!muncher.getIsMoving())
    {
        // Example: Check for keyboard input
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        {
            muncher.startMovement(grid, MuncherDirection::UP);
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
        {
            muncher.startMovement(grid, MuncherDirection::DOWN);
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        {
            muncher.startMovement(grid, MuncherDirection::LEFT);
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        {
            muncher.startMovement(grid, MuncherDirection::RIGHT);
        }
    }
}

#endif
