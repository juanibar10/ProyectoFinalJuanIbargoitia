#include "UI.hpp"
#include <cstdio>

// Loads a font from file
bool UI::loadFont(const std::string& fontPath)
{
	if (m_font.loadFromFile(fontPath)) return true;

	if (fontPath.rfind("assets/", 0) == 0 || fontPath.rfind("assets\\", 0) == 0)
	{
		return false;
	}

	return m_font.loadFromFile(std::string("assets/") + fontPath);
}

float UI::hudHeight()
{
	return 36.f;
}

// Helper to create a text object with given size and color
sf::Text UI::makeText(unsigned int size, sf::Color color) const
{
	sf::Text t;
	t.setFont(m_font);
	t.setCharacterSize(size);
	t.setFillColor(color);
	return t;
}

void UI::drawText(sf::RenderTarget& rt, sf::Text& t, float x, float y)
{
	sf::FloatRect b = t.getLocalBounds();
	t.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
	t.setPosition(x, y);
	rt.draw(t);
}

// Draws the HUD with score and lives
void UI::drawHud(sf::RenderTarget& rt, int score, int lives) const
{
	sf::Text t = makeText(18, sf::Color::White);

	char text[128];
	std::snprintf(text, sizeof(text), "Score: %d   Lives: %d", score, lives);
	text[sizeof(text) - 1] = '\0';

	t.setString(text);
	t.setPosition(12.f, 8.f);
	rt.draw(t);
}

// Draws the main menu, win, and game over screens
void UI::drawScreen(sf::RenderTarget& rt, ScreenState state) const
{
	const sf::Vector2u size = rt.getSize();
	const float w = static_cast<float>(size.x);
	const float h = static_cast<float>(size.y);

	sf::Text title = makeText(52, sf::Color::White);
	sf::Text msg = makeText(22, sf::Color(220, 220, 220));

	switch (state)
	{
		default:
		case ScreenState::Menu:
			{
				title.setString("ARKANOID");
				msg.setString("Press ENTER to Start");
				drawText(rt, title, w * 0.5f, h * 0.38f);
				drawText(rt, msg, w * 0.5f, h * 0.55f);
				return;
			}
		case ScreenState::GameOver:
		case ScreenState::Win:
			{
				title.setString(state == ScreenState::GameOver ? "Game Over" : "You Win!");
				msg.setString("Press ENTER to Restart\nPress ESC to return to Menu");
				drawText(rt, title, w * 0.5f, h * 0.40f);
				drawText(rt, msg, w * 0.5f, h * 0.60f);
				break;
			}
	}
}