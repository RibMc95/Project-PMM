#include <SFML/Graphics.hpp>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include "Maze.h"
#include "PelletGrid.h"
#include "Pellet.h"
#include "Grid.h"
#include "Muncher.h"
#include "Spookies.h"
#include "Spookie_Chase.h"
#include "GameConfig.h"

// Function to draw the maze walls and pellets with clear visual mapping
void drawGame(sf::RenderWindow &window, const Grid &grid, const PelletGrid &pelletGrid)
{
    // Define clear visual mappings for each tile type
    sf::RectangleShape wallBlock(sf::Vector2f(GameConfig::CELL_SIZE, GameConfig::CELL_SIZE));
    wallBlock.setFillColor(sf::Color::Blue); // Classic Pac-Man blue walls

    sf::RectangleShape ghostSpawnBlock(sf::Vector2f(GameConfig::CELL_SIZE, GameConfig::CELL_SIZE));
    ghostSpawnBlock.setFillColor(sf::Color(100, 100, 100)); // Dark gray for ghost spawn area

    sf::RectangleShape playerStartBlock(sf::Vector2f(GameConfig::CELL_SIZE, GameConfig::CELL_SIZE));
    playerStartBlock.setFillColor(sf::Color(0, 100, 0)); // Dark green for player start

    sf::CircleShape pelletShape(3.0f);
    pelletShape.setFillColor(sf::Color::Yellow); // Yellow pellets

    sf::CircleShape powerPelletShape(8.0f);
    powerPelletShape.setFillColor(sf::Color(255, 255, 100)); // Brighter yellow for power pellets

    // Add a subtle background grid for empty spaces (optional)
    sf::RectangleShape emptyBlock(sf::Vector2f(GameConfig::CELL_SIZE, GameConfig::CELL_SIZE));
    emptyBlock.setFillColor(sf::Color(10, 10, 10)); // Very dark background
    emptyBlock.setOutlineColor(sf::Color(20, 20, 20));
    emptyBlock.setOutlineThickness(0.5f);

    for (int y = 0; y < grid.getHeight(); y++)
    {
        for (int x = 0; x < grid.getWidth(); x++)
        {
            float pixelX = x * GameConfig::CELL_SIZE;
            float pixelY = y * GameConfig::CELL_SIZE;

            // Draw base empty space first
            emptyBlock.setPosition(pixelX, pixelY);
            window.draw(emptyBlock);

            // Priority-based rendering (highest priority last)
            if (grid.isWall(x, y))
            {
                wallBlock.setPosition(pixelX, pixelY);
                window.draw(wallBlock);
            }
            // Removed TRANSPARENT and TELEPORT tile drawing
            else if (grid.isGhostSpawn(x, y))
            {
                // Draw green tile for ghost spawn
                sf::RectangleShape ghostSpawnTile(sf::Vector2f(GameConfig::CELL_SIZE, GameConfig::CELL_SIZE));
                ghostSpawnTile.setFillColor(sf::Color(0, 255, 0)); // Bright green
                ghostSpawnTile.setPosition(pixelX, pixelY);
                window.draw(ghostSpawnTile);
            }
            else if (grid.isPlayerStart(x, y))
            {
                // Draw cyan tile for player start
                sf::RectangleShape playerStartTile(sf::Vector2f(GameConfig::CELL_SIZE, GameConfig::CELL_SIZE));
                playerStartTile.setFillColor(sf::Color(0, 255, 255)); // Cyan
                playerStartTile.setPosition(pixelX, pixelY);
                window.draw(playerStartTile);
            }
            else if (grid.getCellType(x, y) == 0 && abs(grid.getFlags(x, y) - 128) < 30)
            {
                // Draw grey tile for scoreboard (if you want to visualize it)
                sf::RectangleShape scoreboardTile(sf::Vector2f(GameConfig::CELL_SIZE, GameConfig::CELL_SIZE));
                scoreboardTile.setFillColor(sf::Color(128, 128, 128));
                scoreboardTile.setPosition(pixelX, pixelY);
                window.draw(scoreboardTile);
            }
            // Draw pellets and power pellets on top if present
            if (pelletGrid.hasPowerPellet(x, y))
            {
                powerPelletShape.setPosition(pixelX + GameConfig::CELL_SIZE / 2 - 8, pixelY + GameConfig::CELL_SIZE / 2 - 8);
                window.draw(powerPelletShape);
            }
            else if (pelletGrid.hasPellet(x, y))
            {
                pelletShape.setPosition(pixelX + GameConfig::CELL_SIZE / 2 - 3, pixelY + GameConfig::CELL_SIZE / 2 - 3);
                window.draw(pelletShape);
            }
        }
    }
}

int main()
{
    // Create window with calculated dimensions
    sf::RenderWindow window(
        sf::VideoMode(GameConfig::WINDOW_WIDTH, GameConfig::WINDOW_HEIGHT),
        "Munch Maze - Pac-Man Style Game");
    window.setFramerateLimit(60);

    // Initialize game objects using PNG file
    Grid grid(GameConfig::GRID_WIDTH, GameConfig::GRID_HEIGHT, false); // Don't auto-initialize

    // Try to load custom maze from PNG first
    std::cout << "\n=== LOADING CUSTOM MAZE ===" << std::endl;
    bool imageLoaded = grid.loadMazeFromImage("practice grid 3.png");

    if (!imageLoaded)
    {
        std::cout << "Failed to load Maze.png, creating default maze..." << std::endl;
        // Fallback to default maze creation
        Maze maze;

        // Validate the maze grid
        std::cout << "\n=== MAZE GRID VALIDATION ===" << std::endl;
        bool isValidMaze = maze.validateGrid();
        std::cout << "Maze grid is " << (isValidMaze ? "VALID" : "INVALID") << std::endl;
        maze.printGridSample(0, 0, 15, 10);   // Print top-left corner
        maze.printGridSample(13, 20, 15, 10); // Print around spawn areas

        // Copy maze walls to grid
        for (int y = 0; y < GameConfig::GRID_HEIGHT; y++)
        {
            for (int x = 0; x < GameConfig::GRID_WIDTH; x++)
            {
                grid.setWall(x, y, maze.isWall(x, y));
            }
        }
    }
    else
    {
        std::cout << "Successfully loaded custom maze from pratice grid.png!" << std::endl;
    }
    std::cout << "============================\n"
              << std::endl;

    // Note: For now, we'll create a simple pelletGrid. You might need to modify PelletGrid
    // to work with Grid instead of Maze, or create pellets based on the Grid data
    PelletGrid pelletGrid(grid);

    // Create Muncher (Pac-Man) at proper spawn point
    Muncher muncher(grid.getPlayerStartX(), grid.getPlayerStartY(), GameConfig::CELL_SIZE);

    // Create Ghosts at green GHOST_SPAWN tiles from the grid
    std::vector<std::pair<int, int>> ghostSpawns;
    for (int y = 0; y < grid.getHeight(); ++y)
    {
        for (int x = 0; x < grid.getWidth(); ++x)
        {
            if (grid.isGhostSpawn(x, y))
            {
                ghostSpawns.emplace_back(x, y);
            }
        }
    }

    std::vector<Ghost> ghosts;
    if (ghostSpawns.size() >= 4)
    {
        ghosts.emplace_back(ghostSpawns[0].first, ghostSpawns[0].second, GhostType::JACK, GameConfig::CELL_SIZE); // Red ghost (Chaser)
        ghosts.emplace_back(ghostSpawns[1].first, ghostSpawns[1].second, GhostType::MIKE, GameConfig::CELL_SIZE); // Blue ghost (Ambusher)
        ghosts.emplace_back(ghostSpawns[2].first, ghostSpawns[2].second, GhostType::SAM, GameConfig::CELL_SIZE);  // Purple ghost (Fickle)
        ghosts.emplace_back(ghostSpawns[3].first, ghostSpawns[3].second, GhostType::WILL, GameConfig::CELL_SIZE); // Yellow ghost (Bashful)
    }
    else
    {
        // Fallback: spawn all at the first found or default location
        int gx = ghostSpawns.empty() ? grid.getGhostSpawnX() : ghostSpawns[0].first;
        int gy = ghostSpawns.empty() ? grid.getGhostSpawnY() : ghostSpawns[0].second;
        ghosts.emplace_back(gx, gy, GhostType::JACK, GameConfig::CELL_SIZE);
        ghosts.emplace_back(gx, gy, GhostType::MIKE, GameConfig::CELL_SIZE);
        ghosts.emplace_back(gx, gy, GhostType::SAM, GameConfig::CELL_SIZE);
        ghosts.emplace_back(gx, gy, GhostType::WILL, GameConfig::CELL_SIZE);
    }

    // Initialize Ghost AI Controller
    GhostAI ghostAI;

    std::cout << "Game initialized with:" << std::endl;
    std::cout << "- Window size: " << GameConfig::WINDOW_WIDTH << "x" << GameConfig::WINDOW_HEIGHT << std::endl;
    std::cout << "- Grid size: " << GameConfig::GRID_WIDTH << "x" << GameConfig::GRID_HEIGHT << std::endl;
    std::cout << "- Cell size: " << GameConfig::CELL_SIZE << "x" << GameConfig::CELL_SIZE << std::endl;
    std::cout << "- Sprite scale: " << GameConfig::SPRITE_SCALE << " (from 100x100 to " << GameConfig::SPRITE_SIZE << "x" << GameConfig::SPRITE_SIZE << ")" << std::endl;
    std::cout << "- Total pellets: " << pelletGrid.countPellets() << std::endl;
    std::cout << "- Total power pellets: " << pelletGrid.countPowerPellets() << std::endl;

    std::cout << "- Muncher initial position: Grid (" << muncher.getPosition().x << "," << muncher.getPosition().y << ")" << std::endl;
    std::cout << "\n=== VISUAL TILE MAPPING ===" << std::endl;
    std::cout << "- WALLS: Blue rectangles (classic Pac-Man style)" << std::endl;
    std::cout << "- PELLETS: Small yellow circles (radius 3px)" << std::endl;
    std::cout << "- POWER PELLETS: Large bright yellow circles (radius 8px)" << std::endl;
    std::cout << "- EMPTY SPACES: Dark background with subtle grid" << std::endl;
    std::cout << "- GHOST SPAWN: Dark gray overlay (debug mode)" << std::endl;
    std::cout << "- PLAYER START: Dark green overlay (debug mode)" << std::endl;
    std::cout << "============================\n"
              << std::endl;

    std::cout << "- AI Personalities:" << std::endl;
    std::cout << "  * JACK (Red) - Chaser: Targets Muncher directly" << std::endl;
    std::cout << "  * MIKE (Blue) - Ambusher: Targets ahead of Muncher" << std::endl;
    std::cout << "  * SAM (Purple) - Fickle: Complex targeting" << std::endl;
    std::cout << "  * WILL (Yellow) - Bashful: Retreats when close" << std::endl;

    // Add these before the game loop
    sf::Clock fruitTimer;
    bool fruitPresent = false;      // fruit initially not present
    bool waitingForRespawn = false; // waiting state after fruit is eaten

    // Fruit types to cycle through
    std::vector<PelletType> fruitTypes =
        {
            PelletType::APPLE,
            PelletType::CHERRY,
            PelletType::STRAWBERRY,
            PelletType::ORANGE,
            PelletType::GRAPEFRUIT,
            PelletType::PANCAKE};
    int currentFruitIndex = 0;
    Pellet fruitPellet(13, 18, fruitTypes[currentFruitIndex]); // fruit spawns at center bottom

    // Game loop
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        // Fruit spawn logic
        if (!fruitPresent && !waitingForRespawn && fruitTimer.getElapsedTime().asSeconds() >= 45.f) // Spawn fruit every 5 seconds
        {
            fruitPresent = true;
            currentFruitIndex = (currentFruitIndex + 1) % fruitTypes.size();
            std::cout << "Spawning fruit type index: " << currentFruitIndex << std::endl;
            fruitPellet.setType(fruitTypes[currentFruitIndex]);
            fruitPellet.reset();
        }

        // Check if fruit is eaten
        if (fruitPresent && muncher.getPosition() == fruitPellet.getPosition() && !fruitPellet.isCollected())
        {
            fruitPellet.collect();
            fruitPresent = false;
            waitingForRespawn = true;
            fruitTimer.restart();
        }

        // After fruit is eaten, wait 5 seconds to respawn
        if (waitingForRespawn && fruitTimer.getElapsedTime().asSeconds() >= 45.f)
        {
            waitingForRespawn = false;
            fruitPresent = true;
            currentFruitIndex = (currentFruitIndex + 1) % fruitTypes.size();
            std::cout << "Respawning fruit type index: " << currentFruitIndex << std::endl;
            fruitPellet.setType(fruitTypes[currentFruitIndex]);
            fruitPellet.reset();
        }

        // Draw fruit if present and not collected
        if (fruitPresent && !fruitPellet.isCollected())
        {
            fruitPellet.draw(window);
        }

        // Update game objects
        muncher.updateAnimation();
        muncher.updateMovement();

        // Update Ghost AI
        ghostAI.update(ghosts, muncher, grid);

        for (auto &ghost : ghosts)
        {
            ghost.updateAnimation();
            ghost.updateMovement();
        }

        // Handle input (basic movement)
        if (!muncher.getIsMoving())
        {
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

        // Check for pellet collisions
        sf::Vector2i muncherPos = muncher.getPosition();
        int gridX = muncherPos.x;
        int gridY = muncherPos.y;

        if (pelletGrid.hasPowerPellet(gridX, gridY))
        {
            std::cout << "Power pellet eaten! Ghosts are now frightened!" << std::endl;
            pelletGrid.setPowerPellet(gridX, gridY, false);
            ghostAI.setFrightened();

            // Set all ghosts to frightened state
            for (auto &ghost : ghosts)
            {
                ghost.setState(GhostState::FRIGHTENED);
            }
        }
        else if (pelletGrid.hasPellet(gridX, gridY))
        {
            std::cout << "Pellet eaten!" << std::endl;
            pelletGrid.setPellet(gridX, gridY, false);
        }

        // Clear window
        window.clear(sf::Color::Black);

        // Draw maze walls and pellets
        drawGame(window, grid, pelletGrid);

        // Draw muncher
        window.draw(muncher.getSprite());

        // Draw ghosts
        for (auto &ghost : ghosts)
        {
            window.draw(ghost.getSprite());
        }

        // Draw fruit if present and not collected
        if (fruitPresent && !fruitPellet.isCollected())
        {
            fruitPellet.draw(window);
        }

        // Display current AI mode (for debugging)
        std::string modeText;
        switch (ghostAI.getCurrentMode())
        {
        case AIMode::SCATTER:
            modeText = "SCATTER";
            break;
        case AIMode::CHASE:
            modeText = "CHASE";
            break;
        case AIMode::FRIGHTENED:
            modeText = "FRIGHTENED";
            break;
        }

        // Simple text display (you might want to add proper font rendering)
        // For now, just print mode changes to console
        static AIMode lastMode = AIMode::SCATTER;
        if (ghostAI.getCurrentMode() != lastMode)
        {
            std::cout << "AI Mode changed to: " << modeText << std::endl;
            lastMode = ghostAI.getCurrentMode();
        }

        // Display
        window.display();
    }

    return 0;
}