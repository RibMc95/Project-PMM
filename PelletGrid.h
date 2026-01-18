#ifndef PELLETGRID_H
#define PELLETGRID_H

#include <vector>
#define PELLETGRID_H

#include <vector>
#include "GameConfig.h"
#include "Grid.h"

// PelletGrid manages pellet and power pellet locations based on Grid flags (from PNG)
class PelletGrid
{
private:
    std::vector<std::vector<bool>> pellets;
    std::vector<std::vector<bool>> powerPellets;
    int width;
    int height;

public:
    PelletGrid(const Grid &grid)
    {
        width = grid.getWidth();
        height = grid.getHeight();
        pellets.resize(height, std::vector<bool>(width, false));
        powerPellets.resize(height, std::vector<bool>(width, false));
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                if (grid.hasFlag(x, y, PELLET))
                    pellets[y][x] = true;
                if (grid.hasFlag(x, y, POWER_PELLET))
                    powerPellets[y][x] = true;
            }
        }
    }

    bool hasPellet(int x, int y) const // Check if there's a pellet at (x, y)
    {
        return isValidPosition(x, y) ? pellets[y][x] : false;
    }
    bool hasPowerPellet(int x, int y) const // Check if there's a power pellet at (x, y)
    {
        return isValidPosition(x, y) ? powerPellets[y][x] : false;
    }
    void setPellet(int x, int y, bool has) // Set or clear a pellet at (x, y)
    {
        if (isValidPosition(x, y))
            pellets[y][x] = has;
    }
    void setPowerPellet(int x, int y, bool has) // Set or clear a power pellet at (x, y)
    {
        if (isValidPosition(x, y))
        {
            powerPellets[y][x] = has;
            if (has)
                pellets[y][x] = false;
        }
    }
    bool isValidPosition(int x, int y) const // Check if (x, y) is within grid bounds
    {
        return (x >= 0 && x < width && y >= 0 && y < height);
    }
    int countPellets() const // Count total pellets
    {
        int count = 0;
        for (int y = 0; y < height; ++y)
            for (int x = 0; x < width; ++x)
                if (pellets[y][x])
                    ++count;
        return count;
    }
    int countPowerPellets() const // Count total power pellets
    {
        int count = 0;
        for (int y = 0; y < height; ++y)
            for (int x = 0; x < width; ++x)
                if (powerPellets[y][x])
                    ++count;
        return count;
    }
};

#endif // PELLETGRID_H