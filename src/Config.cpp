#include "Config.hpp"

#include <fstream>
#include <cstdlib>

Config::Config(const std::string& path) : m_path(path) {}

void Config::trim(std::string& s)
{
    while (!s.empty() && (s.front() == ' ' || s.front() == '\t'))
        s.erase(s.begin());

    while (!s.empty() && (s.back() == ' ' || s.back() == '\t'))
        s.pop_back();
}

bool Config::reload()
{
    m_entries.clear();

    std::ifstream in(m_path);
    if (!in) return false;

    std::string line;
    while (std::getline(in, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        std::size_t eq = line.size();
        for (std::size_t i = 0; i < line.size(); ++i)
        {
            if (line[i] == '=')
            {
                eq = i;
                break;
            }
        }
        if (eq == line.size())
            continue;

        std::string key = line.substr(0, eq);
        std::string value = line.substr(eq + 1);

        trim(key);
        trim(value);

        if (key.empty())
            continue;

        Entry e;
        e.key = key;
        e.value = value;
        m_entries.push_back(e);
    }

    return true;
}

const std::string* Config::findValue(const std::string& key) const
{
    for (std::size_t i = 0; i < m_entries.size(); ++i)
    {
        if (m_entries[i].key == key)
            return &m_entries[i].value;
    }
    return nullptr;
}

int Config::getInt(const std::string& key, int def) const
{
    const std::string* v = findValue(key);
    if (!v) return def;
    return std::atoi(v->c_str());
}

float Config::getFloat(const std::string& key, float def) const
{
    const std::string* v = findValue(key);
    if (!v) return def;
    return static_cast<float>(std::atof(v->c_str()));
}

bool Config::getBool(const std::string& key, bool def) const
{
    const std::string* v = findValue(key);
    if (!v) return def;
    return std::atoi(v->c_str()) != 0;
}

std::string Config::getString(const std::string& key, const std::string& def) const
{
    const std::string* v = findValue(key);
    if (!v) return def;
    return *v;
}

sf::Color Config::getColor(const std::string& key, sf::Color def) const
{
    const std::string* v = findValue(key);
    if (!v) return def;

    std::string s = *v;
    trim(s);

    if (s == "White")  return sf::Color::White;
    if (s == "Black")  return sf::Color::Black;
    if (s == "Yellow") return sf::Color::Yellow;
    if (s == "Red")    return sf::Color::Red;
    if (s == "Green")  return sf::Color::Green;
    if (s == "Cyan")   return sf::Color::Cyan;

    return def;
}

std::string Config::assetPath(const std::string& relative) const
{
    if (relative.empty())
        return "assets";

    return std::string("assets/") + relative;
}