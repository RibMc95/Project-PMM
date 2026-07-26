#ifndef SPRITESHEET_H
#define SPRITESHEET_H

#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>
#include "GameConfig.h"

// ---------------------------------------------------------------------------
// SpriteSheet — the software version of the arcade Pac-Man sprite ROM.
//
// ONE sf::Texture holds every game frame; frames are cut out with
// setTextureRect(). Layout is a grid of 100x100 cells, one category per row:
//
//   row 0 (ghosts) : BODY_UP BODY_DOWN BODY_RIGHT FRIGHT_1 FRIGHT_2 EYES_UP EYES_DOWN EYES_RIGHT
//   row 1 (muncher): IDLE MOVE_1 MOVE_2 DEATH_1 DEATH_2 DEATH_3 DEATH_FINAL
//   row 2 (pellets): PELLET POWER APPLE CHERRY STRAWBERRY ORANGE GRAPEFRUIT PANCAKE
//
// Why so few cells (23, not 37 files): ghosts are ONE grey shape tinted four
// ways; LEFT is a flipped RIGHT; the muncher is one facing rotated four ways.
// ---------------------------------------------------------------------------

enum class GhostFrame // row 0
{
    BODY_UP = 0, // grey, tintable body+eyes (LEFT = flipped RIGHT)
    BODY_DOWN,
    BODY_RIGHT,
    FRIGHT_1, // blue frightened art, used as-is (never tinted)
    FRIGHT_2,
    EYES_UP, // eaten "eyes only", used as-is (LEFT = flipped RIGHT)
    EYES_DOWN,
    EYES_RIGHT
};

enum class MuncherFrame // row 1
{
    IDLE = 0,
    MOVE_1,
    MOVE_2,
    DEATH_1,
    DEATH_2,
    DEATH_3,
    DEATH_FINAL
};

enum class PelletFrame // row 2
{
    PELLET = 0,
    POWER,
    APPLE,
    CHERRY,
    STRAWBERRY,
    ORANGE,
    GRAPEFRUIT,
    PANCAKE
};

class SpriteSheet
{
private:
    static constexpr int CELL = 100; // source art is 100x100 RGBA
    static constexpr int COLS = 8;   // widest row (ghosts & pellets use 8)
    static constexpr int ROWS = 3;
    sf::Texture texture; // the ONE texture everything samples from
    bool ok = false;

    // Luminance-weighted greyscale (alpha preserved). Turning coloured art into
    // neutral grey is what makes a SINGLE shape tintable into any colour.
    static sf::Color toGrey(const sf::Color &c)
    {
        sf::Uint8 v = static_cast<sf::Uint8>(0.30f * c.r + 0.59f * c.g + 0.11f * c.b);
        return sf::Color(v, v, v, c.a); // keep alpha so outlines stay crisp
    }

    // Copy one source PNG into cell (col,row), optionally desaturating it.
    static bool blit(sf::Image &sheet, const std::string &path, int col, int row, bool grey)
    {
        sf::Image src;
        if (!src.loadFromFile(path))
        {
            std::cout << "SpriteSheet: failed to load " << path << std::endl;
            return false;
        }
        const int ox = col * CELL;
        const int oy = row * CELL;
        for (int y = 0; y < CELL; ++y)
        {
            for (int x = 0; x < CELL; ++x)
            {
                sf::Color px = src.getPixel(x, y);
                sheet.setPixel(ox + x, oy + y, grey ? toGrey(px) : px);
            }
        }
        return true;
    }

    static sf::IntRect cell(int col, int row)
    {
        return sf::IntRect(col * CELL, row * CELL, CELL, CELL);
    }

public:
    // Assemble the sheet from the existing loose PNGs, once, at startup.
    // Requires a live graphics context, so construct AFTER the window exists.
    SpriteSheet()
    {
        sf::Image sheet;
        sheet.create(COLS * CELL, ROWS * CELL, sf::Color::Transparent);

        bool good = true;
        // --- row 0: ghosts (one grey shape from Jack, tinted per-ghost later) ---
        good &= blit(sheet, "Spookies/Jack_Up.png", 0, 0, true);
        good &= blit(sheet, "Spookies/Jack_Down.png", 1, 0, true);
        good &= blit(sheet, "Spookies/Jack_Right.png", 2, 0, true);
        good &= blit(sheet, "Spookies/Spookie_Bonus_1.png", 3, 0, false);
        good &= blit(sheet, "Spookies/Spookie_Bonus_2.png", 4, 0, false);
        good &= blit(sheet, "Spookies/eaten_up.png", 5, 0, false);
        good &= blit(sheet, "Spookies/eaten_down.png", 6, 0, false);
        good &= blit(sheet, "Spookies/eaten_right.png", 7, 0, false);
        // --- row 1: muncher (single facing, rotated at draw time) ---
        good &= blit(sheet, "muncher/Munch_idle.png", 0, 1, false);
        good &= blit(sheet, "muncher/Munch_1.png", 1, 1, false);
        good &= blit(sheet, "muncher/Munch_2.png", 2, 1, false);
        good &= blit(sheet, "muncher/Muncher_death_1.png", 3, 1, false);
        good &= blit(sheet, "muncher/Muncher_death_2.png", 4, 1, false);
        good &= blit(sheet, "muncher/Muncher_death_3.png", 5, 1, false);
        good &= blit(sheet, "muncher/Muncher_death_final.png", 6, 1, false);
        // --- row 2: pellets + fruit ---
        good &= blit(sheet, "Objects/Pellet.png", 0, 2, false);
        good &= blit(sheet, "Objects/Power Pellet.png", 1, 2, false);
        good &= blit(sheet, "Objects/apple.png", 2, 2, false);
        good &= blit(sheet, "Objects/cherry.png", 3, 2, false);
        good &= blit(sheet, "Objects/strawberry.png", 4, 2, false);
        good &= blit(sheet, "Objects/orange.png", 5, 2, false);
        good &= blit(sheet, "Objects/grapefruit.png", 6, 2, false);
        good &= blit(sheet, "Objects/pancake.png", 7, 2, false);

        ok = good && texture.loadFromImage(sheet);
        texture.setSmooth(false); // crisp pixels, no blurring

        // Dump the assembled sheet under a distinct name so it never collides
        // with a hand-added SpriteSheet.png on case-insensitive (Windows) filesystems.
        sheet.saveToFile("ghost_sheet_generated.png");

        if (ok)
            std::cout << "SpriteSheet: assembled 23 frames into one texture." << std::endl;
        else
            std::cout << "SpriteSheet: assembly FAILED (missing art?)." << std::endl;
    }

    bool isValid() const { return ok; }
    const sf::Texture &getTexture() const { return texture; }

    // The rectangle that cuts one frame out of the sheet (overloaded per row).
    static sf::IntRect frameRect(GhostFrame f) { return cell(static_cast<int>(f), 0); }
    static sf::IntRect frameRect(MuncherFrame f) { return cell(static_cast<int>(f), 1); }
    static sf::IntRect frameRect(PelletFrame f) { return cell(static_cast<int>(f), 2); }
};

#endif // SPRITESHEET_H
