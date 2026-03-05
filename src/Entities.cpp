#include "Entities.hpp"

// Crea e inicializa una Ball
Entities::Ball Entities::makeBall(float radius, sf::Vector2f pos, sf::Vector2f vel, sf::Color color)
{
    Ball b;
    b.shape.setRadius(radius);
    b.shape.setOrigin(radius, radius);
    b.shape.setPosition(pos);
    b.shape.setFillColor(color);
    b.velocity = vel;
    return b;
}

// Crea e inicializa un Paddle
Entities::Paddle Entities::makePaddle(sf::Vector2f size, sf::Vector2f pos, float speed, sf::Color color)
{
    Paddle p;
    p.shape.setSize(size);
    p.shape.setOrigin(size.x / 2.f, size.y / 2.f);
    p.shape.setPosition(pos);
    p.shape.setFillColor(color);
    p.speed = speed;
    return p;
}

// Crea un Brick
Entities::Brick Entities::makeBrick(sf::Vector2f size, sf::Vector2f pos, sf::Color color, int hp)
{
    Brick br;
    br.shape.setSize(size);
    br.shape.setOrigin(size.x / 2.f, size.y / 2.f);
    br.shape.setPosition(pos);
    br.shape.setFillColor(color);
    br.shape.setOutlineThickness(1.f);
    br.shape.setOutlineColor(sf::Color(40, 40, 40));

    if (hp < 1) hp = 1;
    br.hp = hp;
    br.alive = true;
    return br;
}

// Crea un BreakEffect e inicializa
Entities::BreakEffect Entities::makeBreakEffect(sf::Vector2f pos, sf::Color color, float radius, float durationSeconds)
{
    BreakEffect e;
    e.reset(pos, color, radius, durationSeconds);
    return e;
}

// Crea un PowerUp e inicializa
Entities::PowerUp Entities::makePowerUp(sf::Vector2f pos, sf::Vector2f size, sf::Vector2f vel, sf::Color color)
{
    PowerUp p;
    p.reset(pos, size, vel, color);
    return p;
}


sf::Uint8 Entities::fadeAlpha(float t, float duration)
{
    if (duration <= 0.f) return 0;

    float a = 1.f - (t / duration);
    if (a < 0.f) a = 0.f;
    if (a > 1.f) a = 1.f;

    return static_cast<sf::Uint8>(255.f * a);
}


// Actualiza efectos y borra los que hayan terminado
void Entities::updateBreakEffects(std::vector<BreakEffect>& effects, float dt)
{
    for (std::size_t i = 0; i < effects.size(); )
    {
        effects[i].t += dt;

        if (effects[i].t >= effects[i].duration)
        {
            effects[i] = effects.back();
            effects.pop_back();
            continue;
        }

        ++i;
    }
}

// Dibuja todos los efectos activos
void Entities::drawBreakEffects(sf::RenderTarget& rt, const std::vector<BreakEffect>& effects)
{
    for (std::size_t i = 0; i < effects.size(); ++i)
    {
        const BreakEffect& e = effects[i];

        sf::CircleShape c;
        c.setRadius(e.radius);
        c.setOrigin(e.radius, e.radius);
        c.setPosition(e.pos);

        sf::Color col = e.color;
        col.a = fadeAlpha(e.t, e.duration);

        c.setFillColor(col);
        rt.draw(c);
    }
}