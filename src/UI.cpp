#include "UI.hpp"
#include <sstream>

// Loads a font from file, tries assets/ fallback if needed
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

// Draws text centered at (x, y)
void UI::drawCentered(sf::RenderTarget& rt, sf::Text& t, float x, float y)
{
	auto b = t.getLocalBounds();
	t.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
	t.setPosition(x, y);
	rt.draw(t);
}

// Draws the HUD with score and lives
void UI::drawHud(sf::RenderTarget& rt, int score, int lives) const
{
	auto t = makeText(18, sf::Color::White);
	std::ostringstream oss;
	oss << "Score: " << score << "   Lives: " << lives;
	t.setString(oss.str());
	t.setPosition(12.f, 8.f);
	rt.draw(t);
}

// Draws the main menu, win, and game over screens
void UI::drawScreen(sf::RenderTarget& rt, ScreenState state) const
{
	const auto size = rt.getSize();
	const auto w = static_cast<float>(size.x);
	const auto h = static_cast<float>(size.y);

	auto title = makeText(52, sf::Color::White);
	auto msg = makeText(22, sf::Color(220, 220, 220));

	switch (state)
	{
		default:
		case ScreenState::Menu:
			{
				title.setString("ARKANOID");
				msg.setString("Press ENTER to Start");
				drawCentered(rt, title, w * 0.5f, h * 0.38f);
				drawCentered(rt, msg, w * 0.5f, h * 0.55f);
				return;
			}
		case ScreenState::GameOver:
		case ScreenState::Win:
			{
				title.setString(state == ScreenState::GameOver ? "Game Over" : "You Win!");
				msg.setString("Press ENTER to Restart\nPress ESC to return to Menu");
				drawCentered(rt, title, w * 0.5f, h * 0.40f);
				drawCentered(rt, msg, w * 0.5f, h * 0.60f);
				break;
			}
	}
}