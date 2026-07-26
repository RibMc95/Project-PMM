#ifndef MUNCHER_H
#define MUNCHER_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "Grid.h"
#include "GameConfig.h"
#include "SpriteSheet.h"

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

    // Graphics: one shared sprite sheet (arcade-style) instead of per-state files.
    const SpriteSheet *sheet = nullptr;
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
    Muncher(int startX, int startY, int gridSize, const SpriteSheet &sheetRef);

    // Animation methods
    void applyFrame(); // set texture-rect + rotation from state/direction
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
inline Muncher::Muncher(int startX, int startY, int gridSize, const SpriteSheet &sheetRef)
    : position(startX, startY), renderPosition(startX * gridSize, startY * gridSize),
      direction(MuncherDirection::RIGHT), state(MuncherState::IDLE),
      size(gridSize), sheet(&sheetRef), animationSpeed(0.2f), currentFrame(0),
      movementSpeed(0.3f), isMoving(false)
{
    targetPosition = renderPosition;
    sprite.setOrigin(50.0f, 50.0f); // centre of a 100x100 cell (rotation pivot)
    sprite.setPosition(renderPosition.x + gridSize / 2.0f, renderPosition.y + gridSize / 2.0f);
    applyFrame();
}

// Pick the sheet frame + rotation for the current state/direction.
inline void Muncher::applyFrame()
{
    if (!sheet)
        return;

    sprite.setTexture(sheet->getTexture());

    MuncherFrame frame = MuncherFrame::IDLE;
    switch (state)
    {
    case MuncherState::IDLE:
        frame = MuncherFrame::IDLE;
        break;
    case MuncherState::MOVING:
        frame = (currentFrame % 2 == 0) ? MuncherFrame::MOVE_1 : MuncherFrame::MOVE_2;
        break;
    case MuncherState::DYING:
    {
        const MuncherFrame deaths[4] = {MuncherFrame::DEATH_1, MuncherFrame::DEATH_2,
                                        MuncherFrame::DEATH_3, MuncherFrame::DEATH_FINAL};
        frame = deaths[currentFrame % 4];
        break;
    }
    }

    sprite.setTextureRect(SpriteSheet::frameRect(frame));
    sprite.setScale(GameConfig::CHARACTER_SCALE, GameConfig::CHARACTER_SCALE);

    // Pac is radially symmetric, so ONE facing rotated four ways covers every direction.
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

// Update animation frames
inline void Muncher::updateAnimation()
{
    if (animationClock.getElapsedTime().asSeconds() >= animationSpeed)
    {
        currentFrame++;
        applyFrame();
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
        applyFrame();
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

    // Compute the target tile once.
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

    // Tunnel warp: stepping off the map edge from a teleport tile emerges at the
    // paired teleport tile on the far side, still facing the same way.
    if (!grid.isValidPosition(newGridPos.x, newGridPos.y) && grid.isTeleport(position.x, position.y))
    {
        sf::Vector2i partner = grid.getTeleportPartner(position.x, position.y);
        if (partner.x != -1)
        {
            direction = dir;
            position = partner;
            renderPosition = sf::Vector2f(partner.x * size, partner.y * size);
            targetPosition = renderPosition;
            sprite.setPosition(renderPosition.x + size / 2.0f, renderPosition.y + size / 2.0f);
            isMoving = false; // snapped across, not interpolated
            setState(MuncherState::MOVING);
            return;
        }
    }

    if (!canMove(grid, dir))
    {
        return; // Can't move in that direction
    }

    direction = dir;
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
