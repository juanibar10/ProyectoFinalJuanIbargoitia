#pragma once

#include <SFML/Graphics.hpp>
#include <string>

// Represents the current screen or game state
enum class ScreenState { Menu, Playing, Win, GameOver };

// Handles UI rendering and HUD for the game
class UI
{
public:
	bool loadFont(const std::string& fontPath);

	static float hudHeight();

	void drawHud(sf::RenderTarget& rt, int score, int lives) const;

	void drawScreen(sf::RenderTarget& rt, ScreenState state) const;

private:
	sf::Font m_font;

	sf::Text makeText(unsigned int size, sf::Color color) const;

	// Draws text centered at (x, y)
	static void drawCentered(sf::RenderTarget& rt, sf::Text& t, float x, float y);
};
