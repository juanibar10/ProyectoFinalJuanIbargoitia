#include "Game.hpp"
#include "Collision.hpp"

#include <random>

// Main game logic and loop implementation
Game::Game() : m_window(sf::VideoMode(920, 720), "Arkanoid"), m_level("", &m_config)
{
	m_config.reload();
	// Window and config setup
	const int winW = m_config.getInt("window.width", 920);
	const int winH = m_config.getInt("window.height", 720);
	const std::string title = m_config.getString("window.title", "Arkanoid");
	m_window.create(sf::VideoMode(static_cast<unsigned>(winW), static_cast<unsigned>(winH)), title);
	m_window.setVerticalSyncEnabled(m_config.getBool("window.vsync", true));

	m_bg = m_config.getColor("colors.background", m_bg);

	m_ui.loadFont(m_config.assetPath(m_config.getString("ui.font", "fonts/arial.ttf")));

	{
		m_levelFiles = m_config.getStringArray("level.files");
		m_levelIndex = 0;
	}

	loadLevelByIndex(m_levelIndex);

	const float w = static_cast<float>(m_window.getSize().x);
	const float h = static_cast<float>(m_window.getSize().y);

	const sf::Vector2f paddleSize
	{
		m_config.getFloat("paddle.width", 120.f),
		m_config.getFloat("paddle.height", 18.f)
	};

	const float paddleBottomMargin = m_config.getFloat("paddle.bottomMargin", 50.f);
	const float paddleSpeed = m_config.getFloat("paddle.speed", 540.f);
	const sf::Color paddleColor = m_config.getColor("colors.paddle", sf::Color(230, 230, 230));

	m_paddle = Entities::makePaddle(paddleSize, {w * 0.5f, h - paddleBottomMargin}, paddleSpeed, paddleColor);
}

// Loads a level by its index in the list
void Game::loadLevelByIndex(std::size_t index)
{
	if (m_levelFiles.empty())
	{
		m_level = Level("", &m_config);
		m_levelIndex = 0;
		return;
	}

	if (index >= m_levelFiles.size()) index = 0;
	m_levelIndex = index;

	const std::string rel = m_levelFiles[m_levelIndex];
	m_level = Level(m_config.assetPath(rel), &m_config);
}

// Centers the paddle at the bottom
void Game::respawnPaddle()
{
	const float w = static_cast<float>(m_window.getSize().x);
	const float h = static_cast<float>(m_window.getSize().y);
	const float paddleBottomMargin = m_config.getFloat("paddle.bottomMargin", 50.f);

	m_paddle.shape.setPosition(w * 0.5f, h - paddleBottomMargin);
}

// Spawns a new ball above the paddle
void Game::respawnBall()
{
	m_balls.clear();

	const float ballRadius = m_config.getFloat("ball.radius", 8.f);

	const float offsetY = -(m_paddle.shape.getSize().y / 2.f + ballRadius + 2.f);
	const sf::Vector2f ballPos = m_paddle.shape.getPosition() + sf::Vector2f(0.f, offsetY);

	const sf::Vector2f ballVel = m_config.getVec2f("ball.velocity", {220.f, -260.f});
	const sf::Color ballColor = m_config.getColor("colors.ball", sf::Color(255, 215, 0));

	auto hBall = m_balls.acquire(Entities::makeBall(ballRadius, ballPos, ballVel, ballColor));
	(void)hBall;
}

// Main game loop
void Game::run()
{
	sf::Clock clock;
	while (m_window.isOpen())
	{
		processEvents();
		const float dt = clock.restart().asSeconds();
		update(dt);
		render();
	}
}

// Initializes a new game session
void Game::startNewGame()
{
	m_score = 0;
	m_lives = m_config.getInt("game.lives", 3);

	m_levelIndex = 0;
	loadLevelByIndex(m_levelIndex);

	startRound();
}

// Starts a new round and resets entities
void Game::startRound()
{
	const float w = static_cast<float>(m_window.getSize().x);
	const float hud = UI::hudHeight();

	m_bricks = m_level.loadBricks(w, hud);

	m_breakEffects.clear();
	m_powerUps.clear();

	respawnPaddle();
	respawnBall();

	m_state = ScreenState::Playing;
}

// Handles window and keyboard events
void Game::processEvents()
{
	sf::Event ev{};
	while (m_window.pollEvent(ev))
	{
		if (ev.type == sf::Event::Closed)
		{
			m_window.close();
		}

		if (ev.type == sf::Event::KeyPressed)
		{
			if (ev.key.code == sf::Keyboard::Left) m_moveLeft = true;
			if (ev.key.code == sf::Keyboard::Right) m_moveRight = true;

			if (ev.key.code == sf::Keyboard::Enter)
			{
				if (m_state == ScreenState::Menu || m_state == ScreenState::Win || m_state == ScreenState::GameOver)
				{
					startNewGame();
				}
			}

			if (ev.key.code == sf::Keyboard::Escape)
			{
				if (m_state == ScreenState::Win || m_state == ScreenState::GameOver) m_state = ScreenState::Menu;
			}
		}

		if (ev.type == sf::Event::KeyReleased)
		{
			if (ev.key.code == sf::Keyboard::Left)
				m_moveLeft = false;

			if (ev.key.code == sf::Keyboard::Right)
				m_moveRight = false;
		}
	}
}

// Returns true with a given probability (for power-up drops)
static bool shouldDropPowerUp(float chance01)
{
	if (chance01 <= 0.f) return false;
	if (chance01 >= 1.f) return true;

	thread_local std::mt19937 rng{std::random_device{}()};
	std::uniform_real_distribution dist(0.f, 1.f);
	return dist(rng) < chance01;
}

// Updates game state, handles movement, collisions, scoring, and power-ups
void Game::update(float dt)
{
	if (m_state != ScreenState::Playing) return;

	float dx = 0.f;
	if (m_moveLeft) dx -= m_paddle.speed;
	if (m_moveRight) dx += m_paddle.speed;

	m_paddle.shape.move(dx * dt, 0.f);

	const float w = static_cast<float>(m_window.getSize().x);
	const float h = static_cast<float>(m_window.getSize().y);
	Collision::keepPaddleInBounds(m_paddle, w);

	Collision::StepResult step;
	Collision::stepBalls(m_balls, dt, w, h, UI::hudHeight(), m_paddle, m_bricks, step);

	Collision::stepPowerUps(m_powerUps, dt, h, m_paddle, step);

	m_score += step.bricksDestroyed * m_config.getInt("game.scorePerBrick", 100);

	const float breakRadius = m_config.getFloat("effects.break.radius", 10.f);
	const float breakDuration = m_config.getFloat("effects.break.duration", 0.25f);

	const bool powerupsEnabled = m_config.getBool("powerups.enabled", true);
	const float dropChance = m_config.getFloat("powerups.dropChance", 0.15f);
	const float fallSpeed = m_config.getFloat("powerups.fallSpeed", 140.f);
	const sf::Vector2f puSize = m_config.getVec2f("powerups.size", {22.f, 14.f});
	const sf::Color puColor = m_config.getColor("powerups.color", sf::Color(120, 255, 255));
	const int maxOnScreen = m_config.getInt("powerups.maxOnScreen", 6);

	for (const auto& ev : step.breakEvents)
	{
		sf::Color color = ev.color;
		if (color.a == 0) color = sf::Color::White;

		auto hEffect = m_breakEffects.acquire();
		m_breakEffects.get(hEffect).reset(ev.pos, color, breakRadius, breakDuration);

		if (powerupsEnabled && shouldDropPowerUp(dropChance) && maxOnScreen > 0)
		{
			int activeCount = 0;
			m_powerUps.forEachActive([&](Entities::PowerUp&, ObjectPool<Entities::PowerUp>::Handle)
			{
				++activeCount;
			});

			if (activeCount < maxOnScreen)
			{
				auto hPU = m_powerUps.acquire();
				m_powerUps.get(hPU).reset(ev.pos, puSize,{0.f, fallSpeed}, puColor, Entities::PowerUpType::MultiBall);
			}
		}
	}

	Entities::updateBreakEffects(m_breakEffects, dt);

	if (!step.powerUpCollectEvents.empty())
	{
		const int extraBalls = m_config.getInt("powerups.multiball.extraBalls", 2);
		const float ballRadius = m_config.getFloat("ball.radius", 8.f);
		const sf::Vector2f baseVel = m_config.getVec2f("ball.velocity", {220.f, -260.f});
		const sf::Color ballColor = m_config.getColor("colors.ball", sf::Color(255, 215, 0));

		const float offsetY = -(m_paddle.shape.getSize().y / 2.f + ballRadius + 2.f);
		const sf::Vector2f spawnPos = m_paddle.shape.getPosition() + sf::Vector2f(0.f, offsetY);

		for (const auto& c : step.powerUpCollectEvents)
		{
			if (c.type == Entities::PowerUpType::MultiBall)
			{
				const int n = std::max(0, extraBalls);
				for (int i = 0; i < n; ++i)
				{
					sf::Vector2f v = baseVel;
					const float sign = (i % 2 == 0) ? 1.f : -1.f;
					constexpr float spreadStep = 35.f;
					const float tier = (static_cast<float>(i) * 0.5f) + 1.f;
					const float spread = spreadStep * tier;
					v.x += sign * spread;
					if (v.y > 0.f) v.y = -v.y;

					auto hBall = m_balls.acquire(Entities::makeBall(ballRadius, spawnPos, v, ballColor));
					(void)hBall;
				}
			}
		}
	}

	bool anyBall = false;
	m_balls.forEachActive([&](Entities::Ball&, ObjectPool<Entities::Ball>::Handle)
	{
		anyBall = true;
	});

	if (step.ballLost && !anyBall)
	{
		m_lives--;
		if (m_lives <= 0)
		{
			m_state = ScreenState::GameOver;
		}
		else
		{
			respawnPaddle();
			respawnBall();
		}
	}

	if (Level::isCleared(m_bricks))
	{
		if (!m_levelFiles.empty() && (m_levelIndex + 1) >= m_levelFiles.size())
		{
			m_state = ScreenState::Win;
			return;
		}

		loadLevelByIndex(m_levelIndex + 1);
		startRound();
	}
}

// Renders the current game state and UI
void Game::render()
{
	m_window.clear(m_bg);

	if (m_state == ScreenState::Playing)
	{
		m_ui.drawHud(m_window, m_score, m_lives);

		for (const auto& br : m_bricks) m_window.draw(br.shape);
		m_window.draw(m_paddle.shape);

		m_balls.forEachActive([&](const Entities::Ball& b, ObjectPool<Entities::Ball>::Handle)
		{
			m_window.draw(b.shape);
		});

		m_powerUps.forEachActive([&](const Entities::PowerUp& p, ObjectPool<Entities::PowerUp>::Handle)
		{
			m_window.draw(p.shape);
		});

		Entities::drawBreakEffects(m_window, m_breakEffects);
	}
	else
	{
		m_ui.drawScreen(m_window, m_state);
	}

	m_window.display();
}
