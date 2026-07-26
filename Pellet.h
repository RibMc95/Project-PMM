#ifndef PELLET_H
#define PELLET_H

#include <SFML/Graphics.hpp>
#include "GameConfig.h"
#include "SpriteSheet.h"

enum class PelletType
{
    NORMAL,
    POWER,
    APPLE,
    CHERRY,
    STRAWBERRY,
    GRAPEFRUIT,
    ORANGE,
    PANCAKE
};

class Pellet
{
private:
    sf::Vector2i position;
    PelletType type;
    sf::Sprite sprite;
    const SpriteSheet *sheet = nullptr; // shared unified sprite sheet
    bool collected;

public:
    // Constructor
    Pellet(int x, int y, PelletType pelletType, const SpriteSheet &sheetRef);

    // Methods
    void setType(PelletType pelletType);
    bool isCollected() const { return collected; }
    void collect() { collected = true; }
    void reset() { collected = false; }

    // Getters
    sf::Vector2i getPosition() const { return position; }
    PelletType getType() const { return type; }
    sf::Sprite &getSprite() { return sprite; }

    // Rendering
    void draw(sf::RenderWindow &window) const;
    void drawFruit(sf::RenderWindow &window) const;
};

// Constructor implementation for pellet, power pellet, and fruit types
inline Pellet::Pellet(int x, int y, PelletType pelletType, const SpriteSheet &sheetRef)
    : position(x, y), type(pelletType), sheet(&sheetRef), collected(false)
{
    setType(pelletType);
    sprite.setPosition(x * GameConfig::CELL_SIZE, y * GameConfig::CELL_SIZE);
}

// Set pellet type and corresponding texture
inline void Pellet::setType(PelletType pelletType)
{
    type = pelletType;

    if (sheet)
    {
        PelletFrame frame = PelletFrame::PELLET;
        switch (type)
        {
        case PelletType::NORMAL:
            frame = PelletFrame::PELLET;
            break;
        case PelletType::POWER:
            frame = PelletFrame::POWER;
            break;
        case PelletType::APPLE:
            frame = PelletFrame::APPLE;
            break;
        case PelletType::CHERRY:
            frame = PelletFrame::CHERRY;
            break;
        case PelletType::STRAWBERRY:
            frame = PelletFrame::STRAWBERRY;
            break;
        case PelletType::GRAPEFRUIT:
            frame = PelletFrame::GRAPEFRUIT;
            break;
        case PelletType::ORANGE:
            frame = PelletFrame::ORANGE;
            break;
        case PelletType::PANCAKE:
            frame = PelletFrame::PANCAKE;
            break;
        }
        sprite.setTexture(sheet->getTexture());
        sprite.setTextureRect(SpriteSheet::frameRect(frame));
    }

    // Scale the sprite from 100x100 to appropriate size
    sprite.setScale(GameConfig::SPRITE_SCALE, GameConfig::SPRITE_SCALE);

    // Update sprite position to ensure it's correctly positioned with new texture
    sprite.setPosition(position.x * GameConfig::CELL_SIZE, position.y * GameConfig::CELL_SIZE);
}

// Draw the pellet if not collected
inline void Pellet::draw(sf::RenderWindow &window) const
{
    if (!collected)
    {
        window.draw(sprite);
    }
}

#endif // PELLET_H