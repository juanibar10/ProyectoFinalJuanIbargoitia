#include "Config.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>

// Implementation of Config: loads and parses configuration values from JSON

Config::Config(std::string jsonPath) : m_path(std::move(jsonPath)) {}

bool Config::reload()
{
	std::ifstream in(m_path);
	if (!in)
	{
		m_root = nlohmann::json{};
		return false;
	}

	try
	{
		in >> m_root;
	}
	catch (...)
	{
		m_root = nlohmann::json{};
		return false;
	}
	return true;
}

// Returns the absolute path to an asset inside the assets folder
std::string Config::assetPath(const std::string& relativeInAssets) const
{
	if (relativeInAssets.empty()) return "assets";

	std::string rel = relativeInAssets;
	while (!rel.empty() && (rel.front() == '/' || rel.front() == '\\')) rel.erase(rel.begin());

	return std::string("assets/") + rel;
}

// Finds a node in the JSON using a dotted key (e.g., "window.width")
const nlohmann::json* Config::findNode(const std::string& dottedKey) const
{
	const nlohmann::json* node = &m_root;
	std::size_t start = 0;

	while (start < dottedKey.size())
	{
		const std::size_t dot = dottedKey.find('.', start);
		const std::string token = dottedKey.substr(start, dot == std::string::npos ? std::string::npos : dot - start);

		if (!node->is_object()) return nullptr;
		auto it = node->find(token);
		if (it == node->end()) return nullptr;

		node = &(*it);

		if (dot == std::string::npos) break;
		start = dot + 1;
	}

	return node;
}

int Config::getInt(const std::string& key, int def) const
{
	const auto* n = findNode(key);

	if (!n) return def;
	if (n->is_number_integer()) return n->get<int>();
	if (n->is_number()) return static_cast<int>(n->get<double>());

	return def;
}

float Config::getFloat(const std::string& key, float def) const
{
	const auto* n = findNode(key);

	if (!n) return def;
	if (n->is_number()) return static_cast<float>(n->get<double>());

	return def;
}

bool Config::getBool(const std::string& key, bool def) const
{
	const auto* n = findNode(key);

	if (!n) return def;
	if (n->is_boolean()) return n->get<bool>();

	return def;
}

std::string Config::getString(const std::string& key, const std::string& def) const
{
	const auto* n = findNode(key);

	if (!n) return def;
	if (n->is_string()) return n->get<std::string>();

	return def;
}

// Returns a string array for the given key, or default if not found
std::vector<std::string> Config::getStringArray(const std::string& key, const std::vector<std::string>& def) const
{
	const auto* n = findNode(key);

	if (!n) return def;
	if (!n->is_array()) return def;

	std::vector<std::string> out;
	out.reserve(n->size());

	for (const auto& v : *n)
	{
		if (!v.is_string()) continue;
		std::string s = v.get<std::string>();

		if (s.empty()) continue;
		out.push_back(std::move(s));
	}

	if (out.empty()) return def;
	return out;
}

// Returns a 2D vector for the given key, or default if not found
sf::Vector2f Config::getVec2f(const std::string& key, sf::Vector2f def) const
{
	const auto* n = findNode(key);
	if (!n) return def;

	auto numOr = [](const nlohmann::json& j, float fallback) -> float
	{
		if (!j.is_number()) return fallback;
		return static_cast<float>(j.get<double>());
	};

	if (n->is_array() && n->size() >= 2)
	{
		return {numOr((*n)[0], def.x), numOr((*n)[1], def.y)};
	}

	if (n->is_object())
	{
		auto itX = n->find("x");
		auto itY = n->find("y");

		if (itX != n->end() && itY != n->end())
		{
			return {numOr(*itX, def.x), numOr(*itY, def.y)};
		}
	}

	return def;
}

// Parses a color from a hex string or array
bool Config::parseHexColor(const std::string& s, sf::Color& out)
{
	if (s.size() != 7 && s.size() != 9) return false;
	if (s[0] != '#') return false;

	auto hexToByte = [](char a, char b, sf::Uint8& v) -> bool
	{
		auto hv = [](char c) -> int
		{
			if (c >= '0' && c <= '9') return c - '0';

			c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
			if (c >= 'a' && c <= 'f') return 10 + (c - 'a');

			return -1;
		};

		const int hi = hv(a);
		const int lo = hv(b);

		if (hi < 0 || lo < 0) return false;

		v = static_cast<sf::Uint8>((hi << 4) | lo);
		return true;
	};

	sf::Uint8 r = 255, g = 255, b = 255, a = 255;

	if (!hexToByte(s[1], s[2], r)) return false;
	if (!hexToByte(s[3], s[4], g)) return false;
	if (!hexToByte(s[5], s[6], b)) return false;
	if (s.size() == 9 && !hexToByte(s[7], s[8], a)) return false;

	out = sf::Color(r, g, b, a);
	return true;
}

// Returns a color for the given key, or default if not found
sf::Color Config::getColor(const std::string& key, sf::Color def) const
{
	const auto* n = findNode(key);
	if (!n) return def;

	if (n->is_string())
	{
		sf::Color c;
		if (parseHexColor(n->get<std::string>(), c)) return c;

		return def;
	}

	if (n->is_array() && (n->size() == 3 || n->size() == 4))
	{
		auto val = [&](std::size_t i, int fallback) -> int
		{
			if (i >= n->size()) return fallback;

			const auto& x = (*n)[i];
			if (!x.is_number()) return fallback;

			return static_cast<int>(x.get<double>());
		};

		auto clamp = [](int v) -> sf::Uint8
		{
			if (v < 0) return 0;
			if (v > 255) return 255;

			return static_cast<sf::Uint8>(v);
		};

		const int r = val(0, def.r);
		const int g = val(1, def.g);
		const int b = val(2, def.b);
		const int a = (n->size() == 4) ? val(3, def.a) : def.a;
		return sf::Color(clamp(r), clamp(g), clamp(b), clamp(a));
	}

	return def;
}
