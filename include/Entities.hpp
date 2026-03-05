#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

class Entities
{
public:
    struct Paddle
    {
        sf::RectangleShape shape;
        float speed = 0.f;
    };

    struct Ball
    {
        sf::CircleShape shape;
        sf::Vector2f velocity{0.f, 0.f};
    };

    struct Brick
    {
        sf::RectangleShape shape;
        int hp = 1;
        bool alive = true;
    };

    struct BreakEffect
    {
        sf::Vector2f pos{0.f, 0.f};
        sf::Color color = sf::Color::White;
        float radius = 10.f;
        float t = 0.f;
        float duration = 0.25f;

        void reset(sf::Vector2f p, sf::Color c, float r, float d)
        {
            pos = p;
            color = c;
            radius = r;
            t = 0.f;
            duration = d;
        }
    };

    struct PowerUp
    {
        sf::RectangleShape shape;
        sf::Vector2f velocity{0.f, 0.f};

        void reset(sf::Vector2f pos, sf::Vector2f size, sf::Vector2f vel, sf::Color color)
        {
            shape.setSize(size);
            shape.setOrigin(size.x / 2.f, size.y / 2.f);
            shape.setPosition(pos);
            shape.setFillColor(color);
            shape.setOutlineThickness(1.f);
            shape.setOutlineColor(sf::Color(20, 20, 20));
            velocity = vel;
        }
    };

    static Ball makeBall(float radius, sf::Vector2f pos,sf::Vector2f vel, sf::Color color);

    static Paddle makePaddle(sf::Vector2f size, sf::Vector2f pos,float speed, sf::Color color);

    static Brick makeBrick(sf::Vector2f size, sf::Vector2f pos,sf::Color color, int hp = 1);

    static BreakEffect makeBreakEffect(sf::Vector2f pos, sf::Color color,float radius, float durationSeconds);

    static PowerUp makePowerUp(sf::Vector2f pos, sf::Vector2f size,sf::Vector2f vel, sf::Color color);

    static void updateBreakEffects(std::vector<BreakEffect>& effects, float dt);

    static void drawBreakEffects(sf::RenderTarget& rt,const std::vector<BreakEffect>& effects);

private:
    static sf::Uint8 fadeAlpha(float t, float duration);
};