#include "Entities.hpp"
#include <algorithm>

namespace Entities
{
	// Creates and initializes a Ball
	Ball makeBall(float radius, sf::Vector2f pos, sf::Vector2f vel, sf::Color color)
	{
		Ball b;
		b.shape.setRadius(radius);
		b.shape.setOrigin(radius, radius);
		b.shape.setPosition(pos);
		b.shape.setFillColor(color);
		b.velocity = vel;
		return b;
	}

	// Creates and initializes a Paddle
	Paddle makePaddle(sf::Vector2f size, sf::Vector2f pos, float speed, sf::Color color)
	{
		Paddle p;
		p.shape.setSize(size);
		p.shape.setOrigin(size.x / 2.f, size.y / 2.f);
		p.shape.setPosition(pos);
		p.shape.setFillColor(color);
		p.speed = speed;
		return p;
	}

	// Creates a Brick with 1 HP by default
	Brick makeBrick(sf::Vector2f size, sf::Vector2f pos, sf::Color color)
	{
		return makeBrick(size, pos, color, 1);
	}

	// Creates a Brick with custom HP
	Brick makeBrick(sf::Vector2f size, sf::Vector2f pos, sf::Color color, int hp)
	{
		Brick br;
		br.shape.setSize(size);
		br.shape.setOrigin(size.x / 2.f, size.y / 2.f);
		br.shape.setPosition(pos);
		br.shape.setFillColor(color);
		br.shape.setOutlineThickness(1.f);
		br.shape.setOutlineColor(sf::Color(40, 40, 40));
		br.hp = std::max(1, hp);
		br.alive = true;
		return br;
	}

	// Creates a BreakEffect and initializes it
	BreakEffect makeBreakEffect(sf::Vector2f pos, sf::Color color, float radius, float durationSeconds)
	{
		BreakEffect e;
		e.reset(pos, color, radius, durationSeconds);
		return e;
	}

	// Creates a PowerUp and initializes it
	PowerUp makePowerUp(sf::Vector2f pos, sf::Vector2f size, sf::Vector2f vel, sf::Color color, PowerUpType type)
	{
		PowerUp p;
		p.reset(pos, size, vel, color, type);
		return p;
	}

	// Updates all break effects and releases finished ones
	void updateBreakEffects(ObjectPool<BreakEffect>& effects, float dt)
	{
		effects.forEachActive([&](BreakEffect& e, ObjectPool<BreakEffect>::Handle h)
		{
			if (!e.step(dt)) effects.release(h);
		});
	}

	// Helper for fading out break effects
	static sf::Uint8 fadeAlpha(float t, float duration)
	{
		if (duration <= 0.f) return 0;

		float a = 1.f - (t / duration);

		if (a < 0.f) a = 0.f;
		if (a > 1.f) a = 1.f;

		return static_cast<sf::Uint8>(255.f * a);
	}

	// Draws all active break effects
	void drawBreakEffects(sf::RenderTarget& rt, ObjectPool<BreakEffect>& effects)
	{
		effects.forEachActive([&](BreakEffect& e, ObjectPool<BreakEffect>::Handle)
		{
			sf::CircleShape c;
			c.setRadius(e.radius);
			c.setOrigin(e.radius, e.radius);
			c.setPosition(e.pos);
			auto col = e.color;
			col.a = fadeAlpha(e.t, e.duration);
			c.setFillColor(col);
			rt.draw(c);
		});
	}
}
