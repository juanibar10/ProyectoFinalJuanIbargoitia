#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include "Entities.hpp"
#include "ObjectPool.hpp"

namespace Collision
{
	// Stores collision result between objects
	struct HitResult
	{
		bool hit = false;
		sf::Vector2f normal{0.f, 0.f};
	};

	// Event for when a brick is destroyed
	struct BreakEvent
	{
		sf::Vector2f pos{0.f, 0.f};
		sf::Color color = sf::Color::White;
	};

	// Event for when a power-up is collected
	struct PowerUpCollectEvent
	{
		Entities::PowerUpType type = Entities::PowerUpType::MultiBall;
		sf::Vector2f pos{0.f, 0.f};
	};

	// Aggregates results of a simulation step
	struct StepResult
	{
		int bricksDestroyed = 0;
		bool ballLost = false;
		std::vector<BreakEvent> breakEvents;
		std::vector<PowerUpCollectEvent> powerUpCollectEvents;
	};

	// Ensures paddle stays within window
	void keepPaddleInBounds(Entities::Paddle& paddle, float windowWidth);

	// Updates balls and handles collisions
	void stepBalls(ObjectPool<Entities::Ball>& balls, float dt, float windowWidth, float windowHeight, float hudHeight, const Entities::Paddle& paddle, std::vector<Entities::Brick>& bricks, StepResult& out);

	// Updates power-ups and handles collection
	void stepPowerUps(ObjectPool<Entities::PowerUp>& powerUps, float dt, float windowHeight, const Entities::Paddle& paddle, StepResult& out);
}
