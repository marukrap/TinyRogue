#include "Map.hpp"
#include "../Game/Console.hpp"

#include <algorithm> // for_each

Map::Map(int width, int height)
	: width(width)
	, height(height)
	, tiles(width * height)
{
}

bool Map::isInBounds(int x, int y) const
{
	return x >= 0 && x < width && y >= 0 && y < height;
}

bool Map::isInBounds(const sf::Vector2i& pos) const
{
	return isInBounds(pos.x, pos.y);
}

Tile& Map::at(int x, int y)
{
	return tiles[x + y * width];
}

const Tile& Map::at(int x, int y) const
{
	return tiles[x + y * width];
}

Tile& Map::at(const sf::Vector2i& pos)
{
	return at(pos.x, pos.y);
}

const Tile& Map::at(const sf::Vector2i& pos) const
{
	return at(pos.x, pos.y);
}

void Map::clearExplored(bool flag)
{
	std::for_each(tiles.begin(), tiles.end(), [flag] (auto& tile) { tile.explored = flag; });
}

void Map::draw(Console& console)
{
	for (int y = 0; y < height; ++y)
		for (int x = 0; x < width; ++x)
		{
			const Tile& tile = at(x, y);

			console.setChar(x, y, tile.type, tile.color);
		}
}
