#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include "Entities.hpp"

class Collision
{
public:
	struct StepResult
	{
		int bricksDestroyed = 0;
		bool ballLost = false;

		bool brokeBrick = false;
		sf::Vector2f brokePos{0.f, 0.f};
		sf::Color brokeColor = sf::Color::White;

		bool collectedPowerUp = false;
		sf::Vector2f collectedPos{0.f, 0.f};
	};

	static void keepPaddleInBounds(Entities::Paddle& paddle, float windowWidth);

	static void stepBalls(std::vector<Entities::Ball>& balls, float dt, float windowWidth, float windowHeight, float hudHeight, const Entities::Paddle& paddle, std::vector<Entities::Brick>& bricks, StepResult& out);

	static void stepPowerUps(std::vector<Entities::PowerUp>& powerUps, float dt, float windowHeight, const Entities::Paddle& paddle, StepResult& out);

private:
	static bool circleRectHitBasic(sf::Vector2f center, float radius, const sf::FloatRect& rb, sf::Vector2f& pushOut);

	static bool rectRectHit(const sf::RectangleShape& a, const sf::RectangleShape& b);

	static sf::Uint8 clampByte(int v);
	static sf::Color darker(sf::Color c, float factor);

	static void removeDeadBricks(std::vector<Entities::Brick>& bricks);
};