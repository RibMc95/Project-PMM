#ifndef PELLET_H
#define PELLET_H

#include <SFML/Graphics.hpp>
#include "GameConfig.h"

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
    sf::Texture normalTexture;
    sf::Texture powerTexture;
    sf::Texture appleTexture;
    sf::Texture cherryTexture;
    sf::Texture strawberryTexture;
    sf::Texture grapefruitTexture;
    sf::Texture orangeTexture;
    sf::Texture pancakeTexture;
    bool collected;

public:
    // Constructor
    Pellet(int x, int y, PelletType pelletType = PelletType::NORMAL);

    // Methods
    bool loadTextures();
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
inline Pellet::Pellet(int x, int y, PelletType pelletType) : position(x, y), type(pelletType), collected(false)
{
    loadTextures();
    setType(pelletType);

    // Set position based on grid coordinates
    sprite.setPosition(x * GameConfig::CELL_SIZE, y * GameConfig::CELL_SIZE);
}

// Load pellet textures
inline bool Pellet::loadTextures()
{
    if (!normalTexture.loadFromFile("Objects/Pellet.png") ||
        !powerTexture.loadFromFile("Objects/Power Pellet.png") ||
        !appleTexture.loadFromFile("Objects/Apple.png") ||
        !cherryTexture.loadFromFile("Objects/Cherry.png") ||
        !strawberryTexture.loadFromFile("Objects/Strawberry.png") ||
        !grapefruitTexture.loadFromFile("Objects/Grapefruit.png") ||
        !orangeTexture.loadFromFile("Objects/Orange.png") ||
        !pancakeTexture.loadFromFile("Objects/Pancake.png"))
    {
        return false;
    }
    return true;
}

// Set pellet type and corresponding texture
inline void Pellet::setType(PelletType pelletType)
{
    type = pelletType;

    switch (type)
    {
    case PelletType::NORMAL:
        sprite.setTexture(normalTexture);
        break;
    case PelletType::POWER:
        sprite.setTexture(powerTexture);
        break;
    case PelletType::APPLE:
        sprite.setTexture(appleTexture);
        break;
    case PelletType::CHERRY:
        sprite.setTexture(cherryTexture);
        break;
    case PelletType::STRAWBERRY:
        sprite.setTexture(strawberryTexture);
        break;
    case PelletType::GRAPEFRUIT:
        sprite.setTexture(grapefruitTexture);
        break;
    case PelletType::ORANGE:
        sprite.setTexture(orangeTexture);
        break;
    case PelletType::PANCAKE:
        sprite.setTexture(pancakeTexture);
        break;
    }

    // Scale the sprite from 100x100 to appropriate size
    sprite.setScale(GameConfig::SPRITE_SCALE, GameConfig::SPRITE_SCALE);
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