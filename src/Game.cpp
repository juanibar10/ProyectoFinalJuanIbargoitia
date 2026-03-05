#include "Game.hpp"
#include "Collision.hpp"

#include <cstdlib>

// Main game logic and loop implementation
Game::Game() : m_level("", &m_config)
{
	m_config.reload();

	unsigned winW = m_config.getInt("window.width", 920);
	unsigned winH = m_config.getInt("window.height", 720);
	std::string title = m_config.getString("window.title", "Arkanoid");

	m_window.create(sf::VideoMode(winW, winH), title);

	m_bg = m_config.getColor("colors.background", m_bg);

	m_ui.loadFont(m_config.assetPath(m_config.getString("ui.font", "fonts/arial.ttf")));

	m_levelIndex = 0;
	loadLevelByIndex(m_levelIndex);

	float w = static_cast<float>(m_window.getSize().x);
	float h = static_cast<float>(m_window.getSize().y);

	sf::Vector2f paddleSize(
		m_config.getFloat("paddle.width", 120.f),
		m_config.getFloat("paddle.height", 18.f)
	);

	float paddleBottomMargin = m_config.getFloat("paddle.bottomMargin", 50.f);
	float paddleSpeed = m_config.getFloat("paddle.speed", 540.f);
	sf::Color paddleColor = m_config.getColor("colors.paddle", sf::Color::White);

	m_paddle = Entities::makePaddle(paddleSize, sf::Vector2f(w * 0.5f, h - paddleBottomMargin), paddleSpeed, paddleColor);
}

// Loads a level by its index in the list
void Game::loadLevelByIndex(std::size_t index)
{
	int levelCount = m_config.getInt("level.count", 1);
	if (levelCount <= 0)
	{
		m_level = Level("", &m_config);
		m_levelIndex = 0;
		return;
	}

	if (index >= static_cast<std::size_t>(levelCount))
		index = 0;

	m_levelIndex = index;

	std::string rel = "";
	if (m_levelIndex == 0) rel = m_config.getString("level.file1", "");
	else if (m_levelIndex == 1) rel = m_config.getString("level.file2", "");
	else if (m_levelIndex == 2) rel = m_config.getString("level.file3", "");
	else rel = m_config.getString("level.file1", "");

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

	float vx = m_config.getFloat("ball.velX", 220.f);
	float vy = m_config.getFloat("ball.velY", -260.f);
	sf::Vector2f ballVel(vx, vy);

	const sf::Color ballColor = m_config.getColor("colors.ball", sf::Color::Yellow);

	m_balls.push_back(Entities::makeBall(ballRadius, ballPos, ballVel, ballColor));
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
			if (ev.key.code == sf::Keyboard::Left) m_moveLeft = false;
			if (ev.key.code == sf::Keyboard::Right) m_moveRight = false;
		}
	}
}

// Returns true with a given probability (for power-up drops)
static bool shouldDropPowerUp(float chance)
{
	if (chance <= 0.f) return false;
	if (chance >= 1.f) return true;

	int r = std::rand() % 100;
	return r < (int)(chance * 100.f);
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

	float puW = m_config.getFloat("powerups.width", 22.f);
	float puH = m_config.getFloat("powerups.height", 14.f);
	sf::Vector2f puSize(puW, puH);

	const sf::Color puColor = m_config.getColor("powerups.color", sf::Color::Cyan);
	const int maxOnScreen = m_config.getInt("powerups.maxOnScreen", 6);

	// Si se ha roto al menos un ladrillo este frame, generamos un efecto (usamos el último)
	if (step.brokeBrick)
	{
		sf::Color color = step.brokeColor;
		if (color.a == 0) color = sf::Color::White;

		Entities::BreakEffect e;
		e.reset(step.brokePos, color, breakRadius, breakDuration);
		m_breakEffects.push_back(e);

		if (powerupsEnabled && shouldDropPowerUp(dropChance) && maxOnScreen > 0)
		{
			int activeCount = static_cast<int>(m_powerUps.size());
			if (activeCount < maxOnScreen)
			{
				Entities::PowerUp p;
				p.reset(step.brokePos, puSize, sf::Vector2f(0.f, fallSpeed), puColor);
				m_powerUps.push_back(p);
			}
		}
	}

	Entities::updateBreakEffects(m_breakEffects, dt);


	if (step.collectedPowerUp)
	{
		const int extraBalls = m_config.getInt("powerups.multiball.extraBalls", 2);
		const float ballRadius = m_config.getFloat("ball.radius", 8.f);

		float vx = m_config.getFloat("ball.velX", 220.f);
		float vy = m_config.getFloat("ball.velY", -260.f);
		sf::Vector2f baseVel(vx, vy);

		const sf::Color ballColor = m_config.getColor("colors.ball", sf::Color::Yellow);

		const float offsetY = -(m_paddle.shape.getSize().y / 2.f + ballRadius + 2.f);
		const sf::Vector2f spawnPos = m_paddle.shape.getPosition() + sf::Vector2f(0.f, offsetY);

		int n = extraBalls;
		if (n < 0) n = 0;

		for (int i = 0; i < n; ++i)
		{
			sf::Vector2f v = baseVel;

			float sign = (i % 2 == 0) ? 1.f : -1.f;
			float spreadStep = 35.f;
			float tier = (static_cast<float>(i) * 0.5f) + 1.f;
			float spread = spreadStep * tier;

			v.x += sign * spread;
			if (v.y > 0.f) v.y = -v.y;

			m_balls.push_back(Entities::makeBall(ballRadius, spawnPos, v, ballColor));
		}
	}


	if (step.ballLost && m_balls.empty())
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
		int levelCount = m_config.getInt("level.count", 1);

		if ((int)(m_levelIndex + 1) >= levelCount)
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

		for (std::size_t i = 0; i < m_bricks.size(); ++i) m_window.draw(m_bricks[i].shape);
		m_window.draw(m_paddle.shape);

		for (std::size_t i = 0; i < m_balls.size(); ++i) m_window.draw(m_balls[i].shape);
		for (std::size_t i = 0; i < m_powerUps.size(); ++i) m_window.draw(m_powerUps[i].shape);

		Entities::drawBreakEffects(m_window, m_breakEffects);
	}
	else
	{
		m_ui.drawScreen(m_window, m_state);
	}

	m_window.display();
}
