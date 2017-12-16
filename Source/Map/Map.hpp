#pragma once

#include "Tile.hpp"

#include <SFML/System/Vector2.hpp>

#include <vector>

class Console;

class Map
{
public:
	Map(int width, int height);
	virtual ~Map() = default;

	// NonCopyable
	Map(const Map&) = delete;
	Map& operator=(const Map&) = delete;

	bool isInBounds(int x, int y) const;
	bool isInBounds(const sf::Vector2i& pos) const;

	Tile& at(int x, int y);
	const Tile& at(int x, int y) const;

	Tile& at(const sf::Vector2i& pos);
	const Tile& at(const sf::Vector2i& pos) const;

	void clearExplored(bool flag);

	virtual void draw(Console& console);

public:
	const int width, height;

private:
	std::vector<Tile> tiles;
};
