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
#include "PausableClock.h"

// Function to draw the maze walls and pellets with clear visual mapping
void drawGame(sf::RenderWindow &window, const Grid &grid, const PelletGrid &pelletGrid, const SpriteSheet &sheet)
{
    // Define clear visual mappings for each tile type
    sf::RectangleShape wallBlock(sf::Vector2f(GameConfig::CELL_SIZE, GameConfig::CELL_SIZE));
    wallBlock.setFillColor(sf::Color::Blue); // Classic Pac-Man blue walls

    // Pellet dots drawn from the shared sheet (row 2 cells) instead of circles.
    sf::Sprite pelletSprite(sheet.getTexture(), SpriteSheet::frameRect(PelletFrame::PELLET));
    pelletSprite.setScale(GameConfig::SPRITE_SCALE, GameConfig::SPRITE_SCALE);

    // Power pellet drawn much bigger (and centred) so it's unmistakable next to a normal dot.
    sf::Sprite powerSprite(sheet.getTexture(), SpriteSheet::frameRect(PelletFrame::POWER));
    powerSprite.setScale(GameConfig::SPRITE_SCALE * 2.5f, GameConfig::SPRITE_SCALE * 2.5f);
    powerSprite.setOrigin(50.0f, 50.0f); // centre of the 100x100 cell art so it scales in place

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
            // Ghost-spawn, player-start, tunnel, and scoreboard tiles are NOT
            // drawn — they drive logic only ("the magnet under the table"),
            // leaving a clean all-black-and-blue board. The score TEXT still
            // renders on top of the (now dark) scoreboard region below.
            // Draw pellets and power pellets on top if present
            if (pelletGrid.hasPowerPellet(x, y))
            {
                powerSprite.setPosition(pixelX + GameConfig::CELL_SIZE / 2.0f, pixelY + GameConfig::CELL_SIZE / 2.0f);
                window.draw(powerSprite);
            }
            else if (pelletGrid.hasPellet(x, y))
            {
                pelletSprite.setPosition(pixelX, pixelY);
                window.draw(pelletSprite);
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

    // Build the shared unified sprite sheet once (needs the GL context above).
    SpriteSheet spriteSheet;

    // Initialize game objects using PNG file
    Grid grid(GameConfig::GRID_WIDTH, GameConfig::GRID_HEIGHT, false); // Don't auto-initialize

    // Try to load custom maze from PNG first
    std::cout << "\n=== LOADING CUSTOM MAZE ===" << std::endl;
    bool imageLoaded = grid.loadMazeFromImage(GameConfig::MAZE_IMAGE);

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
    Muncher muncher(grid.getPlayerStartX(), grid.getPlayerStartY(), GameConfig::CELL_SIZE, spriteSheet);

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
        ghosts.emplace_back(ghostSpawns[0].first, ghostSpawns[0].second, GhostType::JACK, GameConfig::CELL_SIZE, spriteSheet); // Red ghost (Chaser)
        ghosts.emplace_back(ghostSpawns[1].first, ghostSpawns[1].second, GhostType::MIKE, GameConfig::CELL_SIZE, spriteSheet); // Blue ghost (Ambusher)
        ghosts.emplace_back(ghostSpawns[2].first, ghostSpawns[2].second, GhostType::SAM, GameConfig::CELL_SIZE, spriteSheet);  // Purple ghost (Fickle)
        ghosts.emplace_back(ghostSpawns[3].first, ghostSpawns[3].second, GhostType::WILL, GameConfig::CELL_SIZE, spriteSheet); // Yellow ghost (Bashful)
    }
    else
    {
        // Fallback: spawn all at the first found or default location
        int gx = ghostSpawns.empty() ? grid.getGhostSpawnX() : ghostSpawns[0].first;
        int gy = ghostSpawns.empty() ? grid.getGhostSpawnY() : ghostSpawns[0].second;
        ghosts.emplace_back(gx, gy, GhostType::JACK, GameConfig::CELL_SIZE, spriteSheet);
        ghosts.emplace_back(gx, gy, GhostType::MIKE, GameConfig::CELL_SIZE, spriteSheet);
        ghosts.emplace_back(gx, gy, GhostType::SAM, GameConfig::CELL_SIZE, spriteSheet);
        ghosts.emplace_back(gx, gy, GhostType::WILL, GameConfig::CELL_SIZE, spriteSheet);
    }

    // Initialize Ghost AI Controller
    GhostAI ghostAI;

    // Apply per-level difficulty (ghost speed + AI mode durations). Re-called after
    // every level-up / death / restart, since ghostAI is rebuilt in those spots.
    auto applyDifficulty = [&](int lvl)
    {
        ghostAI.setLevel(lvl);
        float ghostSpeed = 0.4f - (lvl - 1) * 0.025f; // seconds per tile; lower = faster
        if (ghostSpeed < 0.2f)
            ghostSpeed = 0.2f;
        for (auto &ghost : ghosts)
            ghost.setMovementSpeed(ghostSpeed);
    };
    applyDifficulty(1);

    // Points and lives
    PointSystem points;
    int highScore = 0;
    int frightenedGhostsEaten = 0;      // resets each power pellet; drives the 200/400/800/1600 chain
    bool muncherDying = false;          // true while the death animation plays
    sf::Clock deathClock;               // times the death animation
    const float DEATH_ANIM_TIME = 1.2f; // ~one full death-frame cycle, then a brief hold
    bool gameOver = false;                                 // true once lives hit 0
    MuncherDirection desiredDir = MuncherDirection::RIGHT; // buffered input direction
    bool hasStarted = false;                               // muncher waits for the first key press
    int level = 1;                                         // current level; drives the difficulty ramp
    bool paused = false;                                   // pause menu toggle (P)

    // Load UI font
    sf::Font uiFont;
    bool fontLoaded = uiFont.loadFromFile("Roboto-Black.ttf");
    if (!fontLoaded)
    {
        std::cout << "Failed to load font Roboto-Black.ttf" << std::endl;
    }

    // Find scoreboard bounds from grid tiles
    bool hasScoreboard = false;
    int sbMinX = grid.getWidth();
    int sbMinY = grid.getHeight();
    int sbMaxX = -1;
    int sbMaxY = -1;
    for (int y = 0; y < grid.getHeight(); y++)
    {
        for (int x = 0; x < grid.getWidth(); x++)
        {
            if (grid.isScoreboard(x, y))
            {
                hasScoreboard = true;
                if (x < sbMinX)
                    sbMinX = x;
                if (y < sbMinY)
                    sbMinY = y;
                if (x > sbMaxX)
                    sbMaxX = x;
                if (y > sbMaxY)
                    sbMaxY = y;
            }
        }
    }

    sf::Text scoreText;
    sf::Text highScoreText;
    sf::Text livesText;
    if (fontLoaded)
    {
        scoreText.setFont(uiFont);
        highScoreText.setFont(uiFont);
        livesText.setFont(uiFont);

        scoreText.setFillColor(sf::Color::White);
        highScoreText.setFillColor(sf::Color::White);
        livesText.setFillColor(sf::Color::White);

        scoreText.setCharacterSize(18);
        highScoreText.setCharacterSize(18);
        livesText.setCharacterSize(18);
    }

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
    PausableClock fruitTimer;
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
    Pellet fruitPellet(13, 18, fruitTypes[currentFruitIndex], spriteSheet); // fruit spawns at center bottom

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
            else if (event.type == sf::Event::KeyPressed &&
                     event.key.code == sf::Keyboard::P && !gameOver && !muncherDying)
            {
                paused = !paused; // toggle pause on P (not during death / game over)
                muncher.setPaused(paused);
                for (auto &ghost : ghosts)
                    ghost.setPaused(paused);
                ghostAI.setPaused(paused);
                fruitTimer.setPaused(paused);
            }
        }

        // Pause menu: freeze the scene under a dimmed overlay until P is pressed again.
        if (paused)
        {
            window.clear(sf::Color::Black);
            drawGame(window, grid, pelletGrid, spriteSheet);
            window.draw(muncher.getSprite());
            for (auto &ghost : ghosts)
                window.draw(ghost.getSprite());
            if (fruitPresent && !fruitPellet.isCollected())
                fruitPellet.draw(window);

            sf::RectangleShape dim(sf::Vector2f(GameConfig::WINDOW_WIDTH, GameConfig::WINDOW_HEIGHT));
            dim.setFillColor(sf::Color(0, 0, 0, 160)); // translucent black
            window.draw(dim);

            if (fontLoaded)
            {
                sf::Text pausedText;
                pausedText.setFont(uiFont);
                pausedText.setString("PAUSED\nPress P to resume");
                pausedText.setCharacterSize(40);
                pausedText.setFillColor(sf::Color::White);
                sf::FloatRect pb = pausedText.getLocalBounds();
                pausedText.setOrigin(pb.left + pb.width / 2.0f, pb.top + pb.height / 2.0f);
                pausedText.setPosition(GameConfig::WINDOW_WIDTH / 2.0f, GameConfig::WINDOW_HEIGHT / 2.0f);
                window.draw(pausedText);
            }

            window.display();
            continue;
        }

        // Game over: out of lives. Freeze on a GAME OVER screen; Space restarts
        // the whole run (score & lives reset; high score is kept).
        if (gameOver)
        {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
            {
                points = PointSystem();
                level = 1;
                pelletGrid = PelletGrid(grid);
                muncher.reset(grid.getPlayerStartX(), grid.getPlayerStartY());
                for (auto &ghost : ghosts)
                    ghost.reset();
                ghostAI = GhostAI();
                applyDifficulty(level); // back to level-1 difficulty
                frightenedGhostsEaten = 0;
                gameOver = false;
                hasStarted = false; // fresh game waits for the first key press
            }

            window.clear(sf::Color::Black);
            drawGame(window, grid, pelletGrid, spriteSheet);
            if (fontLoaded)
            {
                sf::Text overText;
                overText.setFont(uiFont);
                overText.setString("GAME OVER\nPress Space to play again");
                overText.setCharacterSize(40);
                overText.setFillColor(sf::Color::Red);
                sf::FloatRect b = overText.getLocalBounds();
                overText.setOrigin(b.left + b.width / 2.0f, b.top + b.height / 2.0f);
                overText.setPosition(GameConfig::WINDOW_WIDTH / 2.0f, GameConfig::WINDOW_HEIGHT / 2.0f);
                window.draw(overText);
            }
            window.display();
            continue;
        }

        // Death pause: freeze gameplay, play the muncher's death animation, then
        // reset everyone to their start tiles. Ghosts are hidden during the death
        // (classic Pac-Man behaviour).
        if (muncherDying)
        {
            muncher.updateAnimation();
            if (deathClock.getElapsedTime().asSeconds() >= DEATH_ANIM_TIME)
            {
                muncherDying = false;
                if (points.getLives() == 0)
                {
                    gameOver = true; // out of lives -> game over (no respawn)
                }
                else
                {
                    muncher.reset(grid.getPlayerStartX(), grid.getPlayerStartY());
                    for (auto &ghost : ghosts)
                        ghost.reset();
                    ghostAI = GhostAI();
                    applyDifficulty(level); // ghostAI was rebuilt -> restore this level's difficulty
                }
            }

            window.clear(sf::Color::Black);
            drawGame(window, grid, pelletGrid, spriteSheet);
            window.draw(muncher.getSprite());
            window.display();
            continue;
        }

        // Fruit spawn logic
        if (!fruitPresent && !waitingForRespawn && fruitTimer.getElapsedTime().asSeconds() >= 45.f) // Spawn the first fruit after 45 seconds
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

            switch (fruitPellet.getType())
            {
            case PelletType::CHERRY:
                points.addPoints(PointSystem::CHERRY_POINTS);
                break;
            case PelletType::STRAWBERRY:
                points.addPoints(PointSystem::STRAWBERRY_POINTS);
                break;
            case PelletType::ORANGE:
                points.addPoints(PointSystem::ORANGE_POINTS);
                break;
            case PelletType::APPLE:
                points.addPoints(PointSystem::APPLE_POINTS);
                break;
            case PelletType::GRAPEFRUIT:
                points.addPoints(PointSystem::GRAPEFRUIT_POINTS);
                break;
            case PelletType::PANCAKE:
                points.addPoints(PointSystem::PANCAKE_POINTS);
                break;
            default:
                break;
            }
        }

        // After fruit is eaten, wait 45 seconds to respawn
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
            ghost.updateMovement(grid, ghosts);
        }

        // Handle input: buffer the latest requested direction, then move Pac-Man
        // style — at each tile take the buffered turn if it's open, otherwise keep
        // heading the current way. Stays still until the first key press.
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        {
            desiredDir = MuncherDirection::UP;
            hasStarted = true;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
        {
            desiredDir = MuncherDirection::DOWN;
            hasStarted = true;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        {
            desiredDir = MuncherDirection::LEFT;
            hasStarted = true;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        {
            desiredDir = MuncherDirection::RIGHT;
            hasStarted = true;
        }

        if (hasStarted && !muncher.getIsMoving())
        {
            if (muncher.canMove(grid, desiredDir))
                muncher.startMovement(grid, desiredDir);             // take the buffered turn
            else
                muncher.startMovement(grid, muncher.getDirection()); // otherwise keep going
        }

        // Check for pellet collisions
        sf::Vector2i muncherPos = muncher.getPosition();
        int gridX = muncherPos.x;
        int gridY = muncherPos.y;

        // Ghost contact = overlap within half a cell, measured on the smooth
        // render positions. This catches head-on passes (mid-tile overlap) and,
        // unlike the old 1.5x bounding-box test, doesn't trigger a full tile away.
        sf::Vector2f muncherRender = muncher.getRenderPosition();
        const float contactDist = GameConfig::CELL_SIZE * 0.5f;
        for (auto &ghost : ghosts)
        {
            sf::Vector2f g = ghost.getRenderPosition();
            float dx = muncherRender.x - g.x;
            float dy = muncherRender.y - g.y;
            bool touching = (dx * dx + dy * dy) < (contactDist * contactDist);

            if (touching && ghost.getState() == GhostState::FRIGHTENED && !ghost.getIsEaten())
            {
                // Escalating chain within one power pellet: 200 -> 400 -> 800 -> 1600.
                static const int frightPoints[4] = {
                    PointSystem::First_Frightened_Ghost_Points,
                    PointSystem::Second_Frightened_Ghost_Points,
                    PointSystem::Third_Frightened_Ghost_Points,
                    PointSystem::Fourth_Frightened_Ghost_Points};
                int idx = frightenedGhostsEaten < 4 ? frightenedGhostsEaten : 3;
                points.addPoints(frightPoints[idx]);
                frightenedGhostsEaten++;
                ghost.setEaten();
            }
            else if (touching && ghost.getState() == GhostState::NORMAL && !muncherDying)
            {
                // Caught by a live ghost: lose a life and start the death animation
                // (the reset happens at the top of the loop when it finishes).
                std::cout << "Muncher caught! Losing a life." << std::endl;
                points.loseLife();
                muncher.setState(MuncherState::DYING);
                muncherDying = true;
                deathClock.restart();
            }
        }

        if (pelletGrid.hasPowerPellet(gridX, gridY))
        {
            std::cout << "Power pellet eaten! Ghosts are now frightened!" << std::endl;
            pelletGrid.setPowerPellet(gridX, gridY, false);
            ghostAI.setFrightened();
            frightenedGhostsEaten = 0; // start a fresh 200/400/800/1600 chain
            points.addPoints(PointSystem::POINTS_PER_POWER_PELLET);

            // Set all ghosts to frightened state
            for (auto &ghost : ghosts)
            {
                ghost.setState(GhostState::FRIGHTENED);
            }
        }
        else if (pelletGrid.hasPellet(gridX, gridY))
        {
            pelletGrid.setPellet(gridX, gridY, false);
            points.addPoints(PointSystem::POINTS_PER_PELLET);
        }

        // Level clear: every pellet eaten -> restart the board, keep score & lives.
        if (pelletGrid.countPellets() == 0 && pelletGrid.countPowerPellets() == 0)
        {
            level++;
            std::cout << "Level cleared! Advancing to level " << level << " (score & lives kept)." << std::endl;
            pelletGrid = PelletGrid(grid); // repopulate all pellets from the map
            muncher.reset(grid.getPlayerStartX(), grid.getPlayerStartY());
            for (auto &ghost : ghosts)
                ghost.reset();
            ghostAI = GhostAI();    // restart the scatter/chase cycle
            applyDifficulty(level); // ramp speed + timings for the new level
        }

        // Clear window
        window.clear(sf::Color::Black);

        // Draw maze walls and pellets
        drawGame(window, grid, pelletGrid, spriteSheet);

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

        // Draw scoreboard text on top of scoreboard tiles
        if (fontLoaded && hasScoreboard)
        {
            if (points.getTotalPoints() > highScore)
            {
                highScore = points.getTotalPoints();
            }

            scoreText.setString("SCORE " + std::to_string(points.getTotalPoints()));
            highScoreText.setString("HIGH " + std::to_string(highScore));
            livesText.setString("LIVES " + std::to_string(points.getLives()));

            float sbLeft = sbMinX * GameConfig::CELL_SIZE;
            float sbTop = sbMinY * GameConfig::CELL_SIZE;
            float paddingX = 6.0f;
            float paddingY = 4.0f;

            scoreText.setPosition(sbLeft + paddingX, sbTop + paddingY);
            highScoreText.setPosition(sbLeft + paddingX, sbTop + paddingY + 22.0f);
            livesText.setPosition(sbLeft + paddingX, sbTop + paddingY + 44.0f);

            window.draw(scoreText);
            window.draw(highScoreText);
            window.draw(livesText);
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