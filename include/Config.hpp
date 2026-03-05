#pragma once

#include <SFML/Graphics/Color.hpp>
#include <string>
#include <vector>

class Config
{
public:
	explicit Config(const std::string& path);

	bool reload();

	int getInt(const std::string& key, int def = 0) const;
	float getFloat(const std::string& key, float def = 0.f) const;
	bool getBool(const std::string& key, bool def = false) const;
	std::string getString(const std::string& key, const std::string& def = "") const;
	sf::Color getColor(const std::string& key, sf::Color def) const;

	std::string assetPath(const std::string& relative) const;

private:
	struct Entry
	{
		std::string key;
		std::string value;
	};

	const std::string* findValue(const std::string& key) const;
	static void trim(std::string& s);

private:
	std::string m_path;
	std::vector<Entry> m_entries;
};