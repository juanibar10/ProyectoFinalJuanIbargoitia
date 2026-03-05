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

	// Loads bricks from the level file and positions them
	std::vector<Entities::Brick> loadBricks(float windowWidth, float hudHeight) const;

	// Checks if all bricks are destroyed
	static bool isCleared(const std::vector<Entities::Brick>& bricks);

private:
	std::string m_levelPath;
	const Config* m_config = nullptr;

	// Returns brick HP based on level character
	int brickHpFor(char c) const;

	// Returns the color for a brick type
	sf::Color brickColorFor(char c) const;
};
