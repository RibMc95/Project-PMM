#ifndef SPOOKIES_H
#define SPOOKIES_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <limits>
#include <cstdlib>
#include <queue>
#include "Grid.h"
#include "GameConfig.h"
#include "SpriteSheet.h"

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
    EATEN,
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

    // Graphics: one shared sprite sheet (arcade-style) instead of per-ghost files.
    const SpriteSheet *sheet = nullptr;
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
    static constexpr float EATEN_DISPLAY_TIME = 8.0f; // Show eaten sprite for 8 seconds

public:
    // Set position directly (for teleportation)
    void setPosition(int x, int y)
    {
        position = sf::Vector2i(x, y);
        renderPosition = sf::Vector2f(x * size, y * size);
        targetPosition = renderPosition;
        sprite.setPosition(renderPosition.x + size / 2.0f, renderPosition.y + size / 2.0f);
        isMoving = false;
    }
    // Constructor
    Ghost(int startX, int startY, GhostType type, int gridSize, const SpriteSheet &sheetRef);

    // Methods
    void applyFrame();          // set texture-rect + tint + flip from state/direction
    sf::Color bodyTint() const; // per-ghost palette colour (Route A palette swap)
    void updateAnimation();
    void updateMovement(const Grid &grid, const std::vector<Ghost> &ghosts);
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
inline Ghost::Ghost(int startX, int startY, GhostType type, int gridSize, const SpriteSheet &sheetRef)
    : position(startX, startY), renderPosition(startX * gridSize, startY * gridSize),
      spawnPosition(startX, startY),
      ghostType(type), direction(GhostDirection::RIGHT), state(GhostState::NORMAL),
      size(gridSize), sheet(&sheetRef), animationSpeed(0.3f), currentFrame(0),
      movementSpeed(0.4f), isMoving(false), isEaten(false)
{
    targetPosition = renderPosition;
    // Centre the origin on the 100x100 cell so a negative X scale flips the
    // sprite in place (LEFT = mirrored RIGHT) without shifting its position.
    sprite.setOrigin(50.0f, 50.0f);
    sprite.setPosition(renderPosition.x + size / 2.0f, renderPosition.y + size / 2.0f);
    applyFrame();
}

// Per-ghost palette colour. This is the Route A "palette register": one grey
// shape is multiplied by this colour to become the coloured ghost.
inline sf::Color Ghost::bodyTint() const
{
    switch (ghostType)
    {
    case GhostType::JACK:
        return sf::Color(255, 0, 0); // red    - Chaser
    case GhostType::MIKE:
        return sf::Color(0, 200, 255); // cyan   - Ambusher
    case GhostType::SAM:
        return sf::Color(190, 90, 255); // purple - Fickle
    case GhostType::WILL:
        return sf::Color(255, 170, 40); // orange - Bashful
    }
    return sf::Color::White;
}

// Pick the sheet frame, tint, and flip that match the current state/direction.
// This replaces all the old per-texture bookkeeping.
inline void Ghost::applyFrame()
{
    if (!sheet)
        return;

    sprite.setTexture(sheet->getTexture());

    GhostFrame frame = GhostFrame::BODY_RIGHT;
    sf::Color tint = sf::Color::White; // White = show the art untinted
    bool flip = false;                 // LEFT is a mirrored RIGHT (arcade hardware flip)

    if (state == GhostState::FRIGHTENED)
    {
        // Two-frame blue blink, never tinted, no facing.
        frame = (currentFrame % 2 == 0) ? GhostFrame::FRIGHT_1 : GhostFrame::FRIGHT_2;
    }
    else if (state == GhostState::RETURNING || isEaten)
    {
        // Eyes only, already coloured, never tinted.
        switch (direction)
        {
        case GhostDirection::UP:
            frame = GhostFrame::EYES_UP;
            break;
        case GhostDirection::DOWN:
            frame = GhostFrame::EYES_DOWN;
            break;
        case GhostDirection::RIGHT:
            frame = GhostFrame::EYES_RIGHT;
            break;
        case GhostDirection::LEFT:
            frame = GhostFrame::EYES_RIGHT;
            flip = true;
            break;
        }
    }
    else // NORMAL
    {
        tint = bodyTint(); // <-- the palette swap happens here
        switch (direction)
        {
        case GhostDirection::UP:
            frame = GhostFrame::BODY_UP;
            break;
        case GhostDirection::DOWN:
            frame = GhostFrame::BODY_DOWN;
            break;
        case GhostDirection::RIGHT:
            frame = GhostFrame::BODY_RIGHT;
            break;
        case GhostDirection::LEFT:
            frame = GhostFrame::BODY_RIGHT;
            flip = true;
            break;
        }
    }

    sprite.setTextureRect(SpriteSheet::frameRect(frame));
    sprite.setColor(tint);

    const float s = GameConfig::CHARACTER_SCALE;
    sprite.setScale(flip ? -s : s, s); // negative X = horizontal mirror about the centred origin
}

// Update animation frames
inline void Ghost::updateAnimation()
{
    if (animationClock.getElapsedTime().asSeconds() >= animationSpeed)
    {
        currentFrame++; // drives the frightened two-frame blink
        applyFrame();
        animationClock.restart();
    }
}

// Update smooth movement between grid positions
inline void Ghost::updateMovement(const Grid &grid, const std::vector<Ghost> &ghosts)
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

            // If eaten and reached a ghost spawn tile, reset to normal
            if (isEaten && (position == spawnPosition || grid.isGhostSpawn(position.x, position.y)))
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

        sprite.setPosition(renderPosition.x + size / 2.0f, renderPosition.y + size / 2.0f);
    }
    else if (isEaten)
    {
        // If eaten and not moving, start moving toward spawn
        if (position != spawnPosition)
        {
            std::cout << "Ghost moving from (" << position.x << "," << position.y << ") to spawn (" << spawnPosition.x << "," << spawnPosition.y << ")" << std::endl;
            GhostDirection bestDir = direction;
            bool foundMove = false;

            GhostDirection directions[] = {
                GhostDirection::UP,
                GhostDirection::DOWN,
                GhostDirection::LEFT,
                GhostDirection::RIGHT};

            auto oppositeOf = [](GhostDirection dir)
            {
                switch (dir)
                {
                case GhostDirection::UP:
                    return GhostDirection::DOWN;
                case GhostDirection::DOWN:
                    return GhostDirection::UP;
                case GhostDirection::LEFT:
                    return GhostDirection::RIGHT;
                case GhostDirection::RIGHT:
                    return GhostDirection::LEFT;
                }
                return GhostDirection::UP;
            };

            auto isPassable = [&](int x, int y)
            {
                if (!grid.isValidPosition(x, y))
                    return false;
                if (!grid.isWall(x, y))
                    return true;
                return grid.isGhostDoor(x, y) || grid.isGhostSpawn(x, y);
            };

            auto isOccupied = [&](const sf::Vector2i &pos)
            {
                if (pos == spawnPosition || grid.isGhostSpawn(pos.x, pos.y))
                    return false;
                for (const auto &other : ghosts)
                {
                    if (&other == this)
                        continue;
                    if (other.getPosition() == pos)
                        return true;
                }
                return false;
            };

            // BFS to the nearest ghost spawn tile (or original spawn position)
            int width = grid.getWidth();
            int height = grid.getHeight();
            std::vector<std::vector<bool>> visited(height, std::vector<bool>(width, false));
            std::vector<std::vector<sf::Vector2i>> prev(height, std::vector<sf::Vector2i>(width, sf::Vector2i(-1, -1)));
            std::queue<sf::Vector2i> q;
            q.push(position);
            visited[position.y][position.x] = true;

            sf::Vector2i target(-1, -1);
            while (!q.empty())
            {
                sf::Vector2i cur = q.front();
                q.pop();

                if (grid.isGhostSpawn(cur.x, cur.y) || cur == spawnPosition)
                {
                    target = cur;
                    break;
                }

                for (GhostDirection dir : directions)
                {
                    sf::Vector2i nextPos = cur;
                    switch (dir)
                    {
                    case GhostDirection::UP:
                        nextPos.y--;
                        break;
                    case GhostDirection::DOWN:
                        nextPos.y++;
                        break;
                    case GhostDirection::LEFT:
                        nextPos.x--;
                        break;
                    case GhostDirection::RIGHT:
                        nextPos.x++;
                        break;
                    }

                    if (!isPassable(nextPos.x, nextPos.y))
                        continue;
                    if (visited[nextPos.y][nextPos.x])
                        continue;

                    visited[nextPos.y][nextPos.x] = true;
                    prev[nextPos.y][nextPos.x] = cur;
                    q.push(nextPos);
                }
            }

            if (target.x != -1 && target != position)
            {
                sf::Vector2i step = target;
                while (prev[step.y][step.x] != position && prev[step.y][step.x].x != -1)
                {
                    step = prev[step.y][step.x];
                }

                if (prev[step.y][step.x] == position)
                {
                    sf::Vector2i delta = step - position;
                    if (delta.x == 1)
                        bestDir = GhostDirection::RIGHT;
                    else if (delta.x == -1)
                        bestDir = GhostDirection::LEFT;
                    else if (delta.y == 1)
                        bestDir = GhostDirection::DOWN;
                    else if (delta.y == -1)
                        bestDir = GhostDirection::UP;

                    if (!isOccupied(step))
                    {
                        foundMove = true;
                    }
                }
            }

            if (!foundMove)
            {
                int bestDistance = std::numeric_limits<int>::max();
                GhostDirection opposite = oppositeOf(direction);

                auto tryPickMove = [&](bool allowReverse)
                {
                    for (GhostDirection dir : directions)
                    {
                        if (!allowReverse && dir == opposite)
                            continue;
                        if (!canMove(grid, dir))
                            continue;

                        sf::Vector2i nextPos = position;
                        switch (dir)
                        {
                        case GhostDirection::UP:
                            nextPos.y--;
                            break;
                        case GhostDirection::DOWN:
                            nextPos.y++;
                            break;
                        case GhostDirection::LEFT:
                            nextPos.x--;
                            break;
                        case GhostDirection::RIGHT:
                            nextPos.x++;
                            break;
                        }

                        if (isOccupied(nextPos))
                            continue;

                        int distance = std::abs(nextPos.x - spawnPosition.x) + std::abs(nextPos.y - spawnPosition.y);

                        if (distance < bestDistance)
                        {
                            bestDistance = distance;
                            bestDir = dir;
                            foundMove = true;
                        }
                    }
                };

                tryPickMove(false);
                if (!foundMove)
                {
                    tryPickMove(true);
                }
            }

            if (foundMove)
            {
                startMovement(grid, bestDir);
            }
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
        applyFrame();
    }
}

// Set movement direction
inline void Ghost::setDirection(GhostDirection newDirection)
{
    direction = newDirection;
    applyFrame(); // refresh texture-rect / flip for the new facing
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

    // Check for walls (ghosts can pass through ghost-door tiles)
    if (!grid.isWall(newPos.x, newPos.y))
        return true;

    return grid.isGhostDoor(newPos.x, newPos.y) &&
           (state == GhostState::RETURNING || state == GhostState::NORMAL);
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