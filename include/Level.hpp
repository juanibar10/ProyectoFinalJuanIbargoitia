#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "Entities.hpp"
#include "Config.hpp"

// Handles loading and parsing a level from file
class Level
{
public:
	explicit Level(std::string levelPath, const Config* config = nullptr);

	std::vector<Entities::Brick> loadBricks(float windowWidth, float hudHeight) const;

	// Checks if all bricks are destroyed
	static bool isCleared(const std::vector<Entities::Brick>& bricks);

private:
	std::string m_levelPath;
	const Config* m_config = nullptr;

	// Returns brick HP based on level character
	int brickHpFor(char c) const
	{
		switch (c)
		{
			case '1': return 2;
			case '2': return 3;
			case '3': return 4;
			case 'B': return 1;
			default: return 1;
		}
	}

	// Returns brick color based on level character
	sf::Color brickColorFor(char c) const;
};
