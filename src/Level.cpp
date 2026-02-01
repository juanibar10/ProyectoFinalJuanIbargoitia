#include "Level.hpp"
#include <algorithm>
#include <fstream>

// Loads a level from file and prepares brick layout
Level::Level(std::string levelPath, const Config* config): m_levelPath(std::move(levelPath)), m_config(config) { }

// Returns the color for a brick type, using config if available
sf::Color Level::brickColorFor(char c) const
{
	sf::Color def;
	switch (c)
	{
		case '1': def = sf::Color(120, 200, 255); break;
		case '2': def = sf::Color(120, 255, 160); break;
		case '3': def = sf::Color(255, 180, 120); break;
		case 'B':
		default:  def = sf::Color(120, 200, 255); break;
	}

	if (m_config)
	{
		const std::string key = std::string("level.colors.") + c;
		return m_config->getColor(key, def);
	}

	return def;
}

// Loads bricks from the level file and positions them
std::vector<Entities::Brick> Level::loadBricks(float windowWidth, float hudHeight) const
{
	std::ifstream in(m_levelPath);
	if (!in) return {};

	std::vector<std::string> lines;
	std::string line;

	while (std::getline(in, line))
	{
		if (!line.empty() && line.back() == '\r') line.pop_back();
		if (line.empty()) continue;
		lines.push_back(line);
	}

	if (lines.empty()) return {};

	std::size_t cols = 0;
	for (const auto& l : lines) cols = std::max(cols, l.size());
	const std::size_t rows = lines.size();

	float sideMargin = 40.f;
	float topMargin = 8.f;
	float padding = 6.f;
	float brickHeight = 22.f;

	if (m_config)
	{
		sideMargin = m_config->getFloat("level.margins.side", sideMargin);
		topMargin = m_config->getFloat("level.margins.top", topMargin);
		padding = m_config->getFloat("level.padding", padding);
		brickHeight = m_config->getFloat("level.brickHeight", brickHeight);
	}

	const float usableW = std::max(10.f, windowWidth - 2.f * sideMargin);
	const auto colsF = static_cast<float>(cols);
	const float brickW = (usableW - padding * (colsF - 1.f)) / colsF;

	std::vector<Entities::Brick> bricks;
	bricks.reserve(rows * cols);

	for (std::size_t r = 0; r < rows; ++r)
	{
		for (std::size_t c = 0; c < lines[r].size(); ++c)
		{
			const char ch = lines[r][c];
			if (ch == ' ' || ch == '0' || ch == '.') continue;

			sf::Vector2f size
			{
				brickW,
				brickHeight
			};

			auto col = static_cast<float>(c);
			auto row = static_cast<float>(r);

			sf::Vector2f pos
			{
				sideMargin + (brickW / 2.f) + col * (brickW + padding),
				hudHeight + topMargin + (brickHeight / 2.f) + row * (brickHeight + padding)
			};

			bricks.push_back(Entities::makeBrick(size, pos, brickColorFor(ch), brickHpFor(ch)));
		}
	}

	return bricks;
}

// Returns true if all bricks are destroyed
bool Level::isCleared(const std::vector<Entities::Brick>& bricks)
{
	return bricks.empty();
}