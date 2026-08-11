#ifndef PAUSABLECLOCK_H
#define PAUSABLECLOCK_H

#include <SFML/System.hpp>

// A pause-aware stand-in for sf::Clock. It supports the two operations the game
// uses on clocks — getElapsedTime() and restart() — plus pause()/resume(), so
// game time can be frozen for a pause menu and continue seamlessly afterwards.
//
// How it works: `base` banks the un-paused time accumulated before the current
// running segment; while running, elapsed = base + (time since last resume).
class PausableClock
{
    sf::Clock clock;                // measures time since the last restart/resume
    sf::Time base = sf::Time::Zero; // un-paused time banked before this segment
    bool running = true;

public:
    sf::Time getElapsedTime() const
    {
        return running ? base + clock.getElapsedTime() : base;
    }

    void restart()
    {
        base = sf::Time::Zero;
        clock.restart();
        running = true;
    }

    void pause()
    {
        if (running)
        {
            base += clock.getElapsedTime(); // bank what has run so far, then stop
            running = false;
        }
    }

    void resume()
    {
        if (!running)
        {
            clock.restart(); // future elapsed is measured from now, added onto base
            running = true;
        }
    }

    void setPaused(bool p) { p ? pause() : resume(); }
};

#endif // PAUSABLECLOCK_H
