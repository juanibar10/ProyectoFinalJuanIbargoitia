#include "Collision.hpp"
#include <cmath>
#include <algorithm>

namespace Collision
{
	static float dot(sf::Vector2f a, sf::Vector2f b)
	{
		return a.x * b.x + a.y * b.y;
	}

	static sf::Vector2f normalize(sf::Vector2f v)
	{
		float len = std::sqrt(v.x * v.x + v.y * v.y);

		if (len <= 0.00001f)
			return {0.f, 0.f};

		return {v.x / len, v.y / len};
	}

	// Checks collision between a circle and a rectangle
	static HitResult circleRectHit(const sf::CircleShape& c, const sf::RectangleShape& r)
	{
		const auto center = c.getPosition();
		const float radius = c.getRadius();
		const auto rb = r.getGlobalBounds();

		const float closestX = std::max(rb.left, std::min(center.x, rb.left + rb.width));
		const float closestY = std::max(rb.top, std::min(center.y, rb.top + rb.height));

		sf::Vector2f diff{center.x - closestX, center.y - closestY};
		const float dist2 = diff.x * diff.x + diff.y * diff.y;

		if (dist2 > radius * radius) return {};

		// Special case: center inside rectangle
		if (std::abs(diff.x) < 0.0001f && std::abs(diff.y) < 0.0001f)
		{
			const float leftPen = std::abs(center.x - rb.left);
			const float rightPen = std::abs((rb.left + rb.width) - center.x);
			const float topPen = std::abs(center.y - rb.top);
			const float bottomPen = std::abs((rb.top + rb.height) - center.y);

			float minPen = leftPen;

			sf::Vector2f n{-1.f, 0.f};

			if (rightPen < minPen)
			{
				minPen = rightPen;
				n = {1.f, 0.f};
			}

			if (topPen < minPen)
			{
				minPen = topPen;
				n = {0.f, -1.f};
			}

			if (bottomPen < minPen)
			{
				n = {0.f, 1.f};
			}

			return {true, n};
		}

		return {true, normalize(diff)};
	}

	// Checks collision between two rectangles
	static bool rectRectHit(const sf::RectangleShape& a, const sf::RectangleShape& b)
	{
		return a.getGlobalBounds().intersects(b.getGlobalBounds());
	}

	// Reflects a vector v over a normal
	static sf::Vector2f reflect(sf::Vector2f v, sf::Vector2f normal)
	{
		normal = normalize(normal);
		return v - 2.f * dot(v, normal) * normal;
	}

	// Keeps the paddle inside the window bounds
	void keepPaddleInBounds(Entities::Paddle& paddle, float windowWidth)
	{
		auto b = paddle.shape.getGlobalBounds();

		if (b.left < 0.f)
			paddle.shape.setPosition(b.width / 2.f, paddle.shape.getPosition().y);

		if (b.left + b.width > windowWidth)
			paddle.shape.setPosition(windowWidth - b.width / 2.f, paddle.shape.getPosition().y);
	}

	// Returns a darker color by a factor
	static sf::Color darker(sf::Color c, float factor)
	{
		factor = std::clamp(factor, 0.f, 1.f);

		auto mul = [&](sf::Uint8 v) -> sf::Uint8
		{
			const int x = static_cast<int>(static_cast<float>(v) * factor);
			return static_cast<sf::Uint8>(std::clamp(x, 0, 255));
		};

		c.r = mul(c.r);
		c.g = mul(c.g);
		c.b = mul(c.b);

		return c;
	}

	// Updates all balls, handles wall, paddle, and brick collisions
	void stepBalls(ObjectPool<Entities::Ball>& balls, float dt, float windowWidth, float windowHeight, float hudHeight, const Entities::Paddle& paddle, std::vector<Entities::Brick>& bricks, StepResult& out)
	{
		balls.forEachActive([&](Entities::Ball& ball, ObjectPool<Entities::Ball>::Handle hBall)
		{
			ball.shape.move(ball.velocity * dt);
			const float r = ball.shape.getRadius();
			auto pos = ball.shape.getPosition();

			// Wall collisions
			if (pos.x - r < 0.f)
			{
				pos.x = r;
				ball.velocity.x = std::abs(ball.velocity.x);
			}

			if (pos.x + r > windowWidth)
			{
				pos.x = windowWidth - r;
				ball.velocity.x = -std::abs(ball.velocity.x);
			}

			if (pos.y - r < hudHeight)
			{
				pos.y = hudHeight + r;
				ball.velocity.y = std::abs(ball.velocity.y);
			}

			if (pos.y - r > windowHeight)
			{
				out.ballLost = true;
				balls.release(hBall);
				return;
			}

			ball.shape.setPosition(pos);

			// Paddle collision
			{
				auto hit = circleRectHit(ball.shape, paddle.shape);

				if (hit.hit && ball.velocity.y > 0.f)
				{
					ball.velocity = reflect(ball.velocity, hit.normal);

					const auto pb = paddle.shape.getGlobalBounds();
					const float dx = (ball.shape.getPosition().x - (pb.left + pb.width / 2.f)) / (pb.width / 2.f);

					ball.velocity.x += dx * 60.f;
					ball.velocity.y = -std::abs(ball.velocity.y);
					ball.shape.move(ball.velocity * dt);
				}
			}

			// Brick collisions
			for (auto& br : bricks)
			{
				if (!br.alive) continue;

				auto hit = circleRectHit(ball.shape, br.shape);

				if (!hit.hit) continue;

				br.hp = std::max(0, br.hp - 1);

				if (br.hp > 0)
				{
					br.shape.setFillColor(darker(br.shape.getFillColor(), 0.85f));
				}

				if (br.hp <= 0)
				{
					const sf::Vector2f breakPos = br.shape.getPosition();
					const sf::Color breakColor = br.shape.getFillColor();
					br.alive = false;
					out.bricksDestroyed++;
					out.breakEvents.push_back({breakPos, breakColor});
				}

				ball.velocity = reflect(ball.velocity, hit.normal);
				ball.shape.move(ball.velocity * dt);
				break;
			}
		});

		// Remove destroyed bricks
		bricks.erase(std::remove_if(bricks.begin(), bricks.end(), [](const Entities::Brick& b) { return !b.alive; }), bricks.end());
	}

	// Updates all power-ups and handles collection
	void stepPowerUps(ObjectPool<Entities::PowerUp>& powerUps, float dt, float windowHeight, const Entities::Paddle& paddle, StepResult& out)
	{
		powerUps.forEachActive([&](Entities::PowerUp& p, ObjectPool<Entities::PowerUp>::Handle h)
		{
			p.shape.move(p.velocity * dt);
			const auto b = p.shape.getGlobalBounds();

			if (b.top > windowHeight)
			{
				powerUps.release(h);
				return;
			}

			if (rectRectHit(p.shape, paddle.shape))
			{
				out.powerUpCollectEvents.push_back({p.type, p.shape.getPosition()});
				powerUps.release(h);
			}
		});
	}
}
