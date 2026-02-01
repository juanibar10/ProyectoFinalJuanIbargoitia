#pragma once

#include <SFML/Graphics.hpp>
#include "ObjectPool.hpp"

namespace Entities
{
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

		void reset(sf::Vector2f p, sf::Color c, float r, float durationSeconds)
		{
			pos = p;
			color = c;
			radius = r;
			t = 0.f;
			duration = durationSeconds;
		}

		bool step(float dt)
		{
			t += dt;
			return t < duration;
		}
	};

	enum class PowerUpType
	{
		MultiBall,
	};

	struct PowerUp
	{
		sf::RectangleShape shape;
		sf::Vector2f velocity{0.f, 0.f};
		PowerUpType type = PowerUpType::MultiBall;

		void reset(sf::Vector2f pos, sf::Vector2f size, sf::Vector2f vel, sf::Color color, PowerUpType t)
		{
			shape.setSize(size);
			shape.setOrigin(size.x / 2.f, size.y / 2.f);
			shape.setPosition(pos);
			shape.setFillColor(color);
			shape.setOutlineThickness(1.f);
			shape.setOutlineColor(sf::Color(20, 20, 20));
			velocity = vel;
			type = t;
		}
	};

	// Factory and utility functions for entities and effects
	Ball makeBall(float radius, sf::Vector2f pos, sf::Vector2f vel, sf::Color color);

	Paddle makePaddle(sf::Vector2f size, sf::Vector2f pos, float speed, sf::Color color);

	Brick makeBrick(sf::Vector2f size, sf::Vector2f pos, sf::Color color);

	Brick makeBrick(sf::Vector2f size, sf::Vector2f pos, sf::Color color, int hp);

	BreakEffect makeBreakEffect(sf::Vector2f pos, sf::Color color, float radius, float durationSeconds);

	PowerUp makePowerUp(sf::Vector2f pos, sf::Vector2f size, sf::Vector2f vel, sf::Color color, PowerUpType type);

	void updateBreakEffects(ObjectPool<BreakEffect>& effects, float dt);

	void drawBreakEffects(sf::RenderTarget& rt, ObjectPool<BreakEffect>& effects);
}
