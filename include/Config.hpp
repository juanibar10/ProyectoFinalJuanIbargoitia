#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>


// Loads and provides access to config values from a JSON file
class Config
{
public:
	explicit Config(std::string jsonPath);
	bool reload();

	int getInt(const std::string& key, int def = 0) const;
	float getFloat(const std::string& key, float def = 0.f) const;
	bool getBool(const std::string& key, bool def = false) const;
	std::string getString(const std::string& key, const std::string& def = {}) const;
	std::vector<std::string> getStringArray(const std::string& key, const std::vector<std::string>& def = {}) const;
	sf::Vector2f getVec2f(const std::string& key, sf::Vector2f def = {0.f, 0.f}) const;
	sf::Color getColor(const std::string& key, sf::Color def = sf::Color::White) const;
	std::string assetPath(const std::string& relativeInAssets) const;


private:
	const nlohmann::json* findNode(const std::string& dottedKey) const;
	static bool parseHexColor(const std::string& s, sf::Color& out);


private:
	std::string m_path;
	nlohmann::json m_root{};
};
