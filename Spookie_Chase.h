#ifndef SPOOKIE_CHASE_H
#define SPOOKIE_CHASE_H

#include "Spookies.h"
#include "Grid.h"
#include "Muncher.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <SFML/System.hpp>

// AI Modes that ghosts cycle through
enum class AIMode
{
    SCATTER,   // Ghosts go to their assigned corners
    CHASE,     // Ghosts actively pursue based on individual logic
    FRIGHTENED // Ghosts move randomly when Pac-Man eats power pellet
};

// Individual ghost behavior types (personalities)
enum class GhostPersonality
{
    CHASER,   // JACK (Red) - Targets Muncher's current position directly
    AMBUSHER, // MIKE (Blue) - Targets ahead of Muncher's direction
    FICKLE,   // SAM (Purple) - Uses complex targeting based on Muncher + Chaser
    BASHFUL   // WILL (Yellow) - Alternates between chase and retreat
};

// Ghost AI Controller class
class GhostAI
{
private:
    // Mode timing (in seconds)
    static constexpr float SCATTER_TIME = 7.0f;
    static constexpr float CHASE_TIME = 20.0f;
    static constexpr float FRIGHTENED_TIME = 10.0f;

    // Targeting parameters
    static constexpr int AMBUSH_DISTANCE = 4;               // Tiles ahead for ambusher
    static constexpr float BASHFUL_RETREAT_DISTANCE = 8.0f; // Distance threshold for bashful retreat

    // Current AI state
    AIMode currentMode;
    sf::Clock modeTimer;
    sf::Clock frightenedTimer;
    bool modeJustChanged;

    // Corner positions for scatter mode (each ghost has a specific corner)
    sf::Vector2i scatterCorners[4];

    // Reference to the chaser ghost for fickle ghost calculations
    Ghost *chaserGhost;

public:
    GhostAI();

    // Main update function
    void update(std::vector<Ghost> &ghosts, const Muncher &muncher, const Grid &grid);

    // Mode management
    void updateMode();
    void setFrightened();
    AIMode getCurrentMode() const { return currentMode; }
    void returnToCurrentMode();
    AIMode getPreviousMode() const;

    // Individual ghost AI behaviors
    sf::Vector2i getTargetTile(const Ghost &ghost, const Muncher &muncher, const Grid &grid);
    GhostDirection getBestDirection(const Ghost &ghost, sf::Vector2i targetTile, const Grid &grid);

    // Specific targeting strategies
    sf::Vector2i getChaserTarget(const Muncher &muncher);
    sf::Vector2i getAmbusherTarget(const Muncher &muncher);
    sf::Vector2i getFickleTarget(const Muncher &muncher, const Ghost &chaserGhost);
    sf::Vector2i getBashfulTarget(const Ghost &bashfulGhost, const Muncher &muncher);

    // Utility functions
    float calculateDistance(sf::Vector2i pos1, sf::Vector2i pos2);
    GhostPersonality getPersonality(GhostType type);
    std::vector<GhostDirection> getValidDirections(const Ghost &ghost, const Grid &grid);
    sf::Vector2i getDirectionOffset(GhostDirection direction);
    GhostDirection getOppositeDirection(GhostDirection direction);
};

// Constructor - Initialize scatter corners and AI state
inline GhostAI::GhostAI()
    : currentMode(AIMode::SCATTER), modeJustChanged(false), chaserGhost(nullptr)
{
    // Set scatter corners for 28x31 authentic Pac-Man grid
    // These should be the four corners of your maze
    scatterCorners[0] = sf::Vector2i(1, 1);   // JACK (Red) - Top Left
    scatterCorners[1] = sf::Vector2i(26, 1);  // MIKE (Blue) - Top Right
    scatterCorners[2] = sf::Vector2i(1, 29);  // SAM (Purple) - Bottom Left
    scatterCorners[3] = sf::Vector2i(26, 29); // WILL (Yellow) - Bottom Right

    modeTimer.restart();
}

// Main update function - called every frame
inline void GhostAI::update(std::vector<Ghost> &ghosts, const Muncher &muncher, const Grid &grid)
{
    // Store previous mode to detect transitions
    AIMode previousMode = currentMode;

    // Update AI mode timing
    updateMode();

    // If we just exited frightened mode, return non-returning ghosts to normal state
    if (previousMode == AIMode::FRIGHTENED && currentMode != AIMode::FRIGHTENED)
    {
        for (auto &ghost : ghosts)
        {
            // Keep ghosts in RETURNING state, only reset others to NORMAL
            if (ghost.getState() != GhostState::RETURNING)
            {
                ghost.setState(GhostState::NORMAL);
            }
        }
    }

    // Find the chaser ghost for fickle calculations
    for (auto &ghost : ghosts)
    {
        if (ghost.getGhostType() == GhostType::JACK)
        {
            chaserGhost = &ghost;
            break;
        }
    }

    // Update each ghost's AI
    for (auto &ghost : ghosts)
    {
        // Returning ghosts should head back to spawn without AI steering.
        if (ghost.getState() == GhostState::RETURNING || ghost.getIsEaten())
            continue;

        // Skip if ghost is currently moving
        if (ghost.getIsMoving())
            continue;

        sf::Vector2i targetTile;

        // Determine target based on current mode
        if (currentMode == AIMode::FRIGHTENED)
        {
            // Random movement when frightened
            std::vector<GhostDirection> validDirs = getValidDirections(ghost, grid);
            if (!validDirs.empty())
            {
                // Choose random valid direction (avoid going backwards if possible)
                GhostDirection opposite = getOppositeDirection(ghost.getDirection());
                std::vector<GhostDirection> preferredDirs;

                for (GhostDirection dir : validDirs)
                {
                    if (dir != opposite)
                        preferredDirs.push_back(dir);
                }

                if (!preferredDirs.empty())
                {
                    int randomIndex = rand() % preferredDirs.size();
                    ghost.startMovement(grid, preferredDirs[randomIndex]);
                }
                else if (!validDirs.empty())
                {
                    int randomIndex = rand() % validDirs.size();
                    ghost.startMovement(grid, validDirs[randomIndex]);
                }
            }
        }
        else
        {
            // Get target tile based on current mode and ghost personality
            targetTile = getTargetTile(ghost, muncher, grid);

            // Get best direction to reach target
            GhostDirection bestDir = getBestDirection(ghost, targetTile, grid);

            // Move ghost
            ghost.startMovement(grid, bestDir);
        }
    }
}

// Update AI mode based on timing
inline void GhostAI::updateMode()
{
    float elapsed = modeTimer.getElapsedTime().asSeconds();
    AIMode previousMode = currentMode;

    // Handle frightened mode (overrides normal cycle)
    if (currentMode == AIMode::FRIGHTENED)
    {
        if (frightenedTimer.getElapsedTime().asSeconds() >= FRIGHTENED_TIME)
        {
            currentMode = AIMode::CHASE; // Return to chase after frightened
            modeTimer.restart();
        }
        return;
    }

    // Normal mode cycling
    switch (currentMode)
    {
    case AIMode::SCATTER:
        if (elapsed >= SCATTER_TIME)
        {
            currentMode = AIMode::CHASE;
            modeTimer.restart();
        }
        break;

    case AIMode::CHASE:
        if (elapsed >= CHASE_TIME)
        {
            currentMode = AIMode::SCATTER;
            modeTimer.restart();
        }
        break;

    default:
        break;
    }

    modeJustChanged = (previousMode != currentMode);
}

// Set frightened mode (called when power pellet is eaten)
inline void GhostAI::setFrightened()
{
    currentMode = AIMode::FRIGHTENED;
    frightenedTimer.restart();
    modeJustChanged = true;
}

inline AIMode GhostAI::getPreviousMode() const
{
    // This function can be expanded to track previous mode if needed
    // For now, it simply returns CHASE as a default
    return AIMode::CHASE;
}

// return to current mode after power pellet effect ends
inline void GhostAI::returnToCurrentMode()
{
    currentMode = AIMode::CHASE; // or SCATTER based on previous state
    modeTimer.restart();
}

// Get target tile for a ghost based on mode and personality
inline sf::Vector2i GhostAI::getTargetTile(const Ghost &ghost, const Muncher &muncher, const Grid &grid)
{
    // Suppress unused parameter warning for grid (may be used in future enhancements)
    (void)grid;

    if (currentMode == AIMode::SCATTER)
    {
        // Return to assigned corner
        int ghostIndex = static_cast<int>(ghost.getGhostType());
        return scatterCorners[ghostIndex];
    }
    else // CHASE mode
    {
        GhostPersonality personality = getPersonality(ghost.getGhostType());

        switch (personality)
        {
        case GhostPersonality::CHASER:
            return getChaserTarget(muncher);

        case GhostPersonality::AMBUSHER:
            return getAmbusherTarget(muncher);

        case GhostPersonality::FICKLE:
            if (chaserGhost)
                return getFickleTarget(muncher, *chaserGhost);
            else
                return getChaserTarget(muncher); // Fallback

        case GhostPersonality::BASHFUL:
            return getBashfulTarget(ghost, muncher);

        default:
            return getChaserTarget(muncher);
        }
    }
}

// Get best direction to reach target tile
inline GhostDirection GhostAI::getBestDirection(const Ghost &ghost, sf::Vector2i targetTile, const Grid &grid)
{
    std::vector<GhostDirection> validDirs = getValidDirections(ghost, grid);

    if (validDirs.empty())
        return ghost.getDirection(); // Can't move anywhere

    // Don't go backwards unless it's the only option
    GhostDirection opposite = getOppositeDirection(ghost.getDirection());
    std::vector<GhostDirection> preferredDirs;

    for (GhostDirection dir : validDirs)
    {
        if (dir != opposite)
            preferredDirs.push_back(dir);
    }

    // Use preferred directions if available, otherwise use all valid
    std::vector<GhostDirection> &dirsToCheck = preferredDirs.empty() ? validDirs : preferredDirs;

    // Find direction that gets closest to target
    GhostDirection bestDir = dirsToCheck[0];
    float bestDistance = std::numeric_limits<float>::max();

    for (GhostDirection dir : dirsToCheck)
    {
        sf::Vector2i testPos = ghost.getPosition() + getDirectionOffset(dir);
        float distance = calculateDistance(testPos, targetTile);

        if (distance < bestDistance)
        {
            bestDistance = distance;
            bestDir = dir;
        }
    }

    return bestDir;
}

// CHASER (JACK/Red) - Targets Muncher's current position directly
inline sf::Vector2i GhostAI::getChaserTarget(const Muncher &muncher)
{
    return sf::Vector2i(muncher.getPosition().x, muncher.getPosition().y);
}

// AMBUSHER (MIKE/Blue) - Targets 4 tiles ahead of Muncher's direction
inline sf::Vector2i GhostAI::getAmbusherTarget(const Muncher &muncher)
{
    sf::Vector2i muncherPos(muncher.getPosition().x, muncher.getPosition().y);
    sf::Vector2i offset(0, 0);

    // Get offset based on Muncher's direction
    switch (muncher.getDirection())
    {
    case MuncherDirection::UP:
        // Original bug: offset both up and left when facing up
        offset = sf::Vector2i(-AMBUSH_DISTANCE / 2, -AMBUSH_DISTANCE / 2);
        break;
    case MuncherDirection::DOWN:
        offset = sf::Vector2i(0, AMBUSH_DISTANCE);
        break;
    case MuncherDirection::LEFT:
        offset = sf::Vector2i(-AMBUSH_DISTANCE, 0);
        break;
    case MuncherDirection::RIGHT:
        offset = sf::Vector2i(AMBUSH_DISTANCE, 0);
        break;
    }

    return muncherPos + offset;
}

// FICKLE (SAM/Purple) - Complex targeting based on Muncher and Chaser positions
inline sf::Vector2i GhostAI::getFickleTarget(const Muncher &muncher, const Ghost &chaserGhost)
{
    sf::Vector2i muncherPos(muncher.getPosition().x, muncher.getPosition().y);
    sf::Vector2i chaserPos = chaserGhost.getPosition();

    // Get point 2 tiles ahead of Muncher
    sf::Vector2i offset(0, 0);
    switch (muncher.getDirection())
    {
    case MuncherDirection::UP:
        offset = sf::Vector2i(0, -2);
        break;
    case MuncherDirection::DOWN:
        offset = sf::Vector2i(0, 2);
        break;
    case MuncherDirection::LEFT:
        offset = sf::Vector2i(-2, 0);
        break;
    case MuncherDirection::RIGHT:
        offset = sf::Vector2i(2, 0);
        break;
    }

    sf::Vector2i midPoint = muncherPos + offset;

    // Vector from chaser to midpoint
    sf::Vector2i vector = midPoint - chaserPos;

    // Double the vector for fickle's target
    return midPoint + vector;
}

// BASHFUL (WILL/Yellow) - Alternates between chase and retreat
inline sf::Vector2i GhostAI::getBashfulTarget(const Ghost &bashfulGhost, const Muncher &muncher)
{
    sf::Vector2i ghostPos = bashfulGhost.getPosition();
    sf::Vector2i muncherPos(muncher.getPosition().x, muncher.getPosition().y);

    float distance = calculateDistance(ghostPos, muncherPos);

    if (distance < BASHFUL_RETREAT_DISTANCE)
    {
        // Too close - retreat to scatter corner
        return scatterCorners[static_cast<int>(bashfulGhost.getGhostType())];
    }
    else
    {
        // Far enough - chase normally
        return muncherPos;
    }
}

// Calculate Euclidean distance between two points
inline float GhostAI::calculateDistance(sf::Vector2i pos1, sf::Vector2i pos2)
{
    float dx = static_cast<float>(pos1.x - pos2.x);
    float dy = static_cast<float>(pos1.y - pos2.y);
    return std::sqrt(dx * dx + dy * dy);
}

// Get personality based on ghost type
inline GhostPersonality GhostAI::getPersonality(GhostType type)
{
    switch (type)
    {
    case GhostType::JACK:
        return GhostPersonality::CHASER; // Red
    case GhostType::MIKE:
        return GhostPersonality::AMBUSHER; // Blue
    case GhostType::SAM:
        return GhostPersonality::FICKLE; // Purple
    case GhostType::WILL:
        return GhostPersonality::BASHFUL; // Yellow
    default:
        return GhostPersonality::CHASER;
    }
}

// Get all valid movement directions for a ghost
inline std::vector<GhostDirection> GhostAI::getValidDirections(const Ghost &ghost, const Grid &grid)
{
    std::vector<GhostDirection> validDirs;

    std::vector<GhostDirection> allDirs = {
        GhostDirection::UP, GhostDirection::DOWN,
        GhostDirection::LEFT, GhostDirection::RIGHT};

    for (GhostDirection dir : allDirs)
    {
        if (ghost.canMove(grid, dir))
        {
            validDirs.push_back(dir);
        }
    }

    return validDirs;
}

// Get offset vector for a direction
inline sf::Vector2i GhostAI::getDirectionOffset(GhostDirection direction)
{
    switch (direction)
    {
    case GhostDirection::UP:
        return sf::Vector2i(0, -1);
    case GhostDirection::DOWN:
        return sf::Vector2i(0, 1);
    case GhostDirection::LEFT:
        return sf::Vector2i(-1, 0);
    case GhostDirection::RIGHT:
        return sf::Vector2i(1, 0);
    default:
        return sf::Vector2i(0, 0);
    }
}

// Get opposite direction
inline GhostDirection GhostAI::getOppositeDirection(GhostDirection direction)
{
    switch (direction)
    {
    case GhostDirection::UP:
        return GhostDirection::DOWN;
    case GhostDirection::DOWN:
        return GhostDirection::UP;
    case GhostDirection::LEFT:
        return GhostDirection::RIGHT;
    case GhostDirection::RIGHT:
        return GhostDirection::LEFT;
    default:
        return direction;
    }
}

#endif // SPOOKIE_CHASE_H