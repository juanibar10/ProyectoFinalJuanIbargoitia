#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include "Entities.hpp"
#include "Level.hpp"
#include "UI.hpp"
#include "Config.hpp"

class Game
{
public:
	Game();

	void run();

private:
	void processEvents(); // Handles input and window events

	void update(float dt); // Updates game state

	void render(); // Draws everything

	void startNewGame(); // Initializes a new game

	void startRound(); // Starts a new round

	void respawnPaddle();

	void respawnBall();

	void loadLevelByIndex(std::size_t index); // Loads a level by index

private:
	sf::RenderWindow m_window;

	sf::Color m_bg{30, 30, 40};

	Config m_config{"assets/config/game.txt"};

	UI m_ui;

	Level m_level;

	std::size_t m_levelIndex = 0;

	ScreenState m_state = ScreenState::Menu;

	Entities::Paddle m_paddle;

	std::vector<Entities::Ball> m_balls;

	std::vector<Entities::Brick> m_bricks;

	std::vector<Entities::BreakEffect> m_breakEffects;

	std::vector<Entities::PowerUp> m_powerUps;

	int m_score = 0;

	int m_lives = 3;

	bool m_moveLeft = false;

	bool m_moveRight = false;
};
