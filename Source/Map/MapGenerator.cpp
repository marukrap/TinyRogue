#include "MapGenerator.hpp"
#include "../World/Level.hpp"
#include "../Utility/Rng.hpp"
#include "../Utility/Direction.hpp"
#include "../Utility/Utility.hpp"
#include "../Graphics/Color.hpp"

#include <utility> // swap
#include <cmath> // abs
#include <climits> // INT_MAX
#include <cassert>

namespace
{
	int even(int number)
	{
		return number / 2 * 2;
	}
}

void MapGenerator::generate(Level& map, Rng& rng, int depth)
{
	this->map = &map;
	this->rng = &rng;
	this->depth = depth;

	onGenerate();

	for (int y = 0; y < map.height; ++y)
		for (int x = 0; x < map.width; ++x)
		{
			Tile& tile = map.at(x, y);

			switch (tile.type)
			{
			case Tile::Unused:
				tile.passable = false;
				tile.transparent = false;
				break;

			case Tile::Wall:
				tile.passable = false;
				tile.transparent = false;
				tile.color = Color::LightGray;
				break;

			case Tile::Floor:
				tile.passable = true;
				tile.transparent = true;
				tile.color = Color::LightGray * sf::Color(100, 100, 100);
				break;

			case Tile::Corridor:
				tile.type = Tile::Floor;
				tile.passable = true;
				tile.transparent = false;
				tile.color = Color::LightGray * sf::Color(100, 100, 100);
				break;

			case Tile::Door:
				tile.passable = true;
				tile.transparent = false;
				tile.color = Color::Brown;
				break;

			case Tile::UpStairs:
			case Tile::DownStairs:
				tile.passable = true;
				tile.transparent = true;
				tile.color = Color::Cyan;
				break;
			}
		}
}

void MapGenerator::fill(Tile::Type tileType)
{
	for (int y = 0; y < map->height; ++y)
		for (int x = 0; x < map->width; ++x)
			map->at(x, y).type = tileType;
}

void MapGenerator::relaxation(std::vector<Point>& points)
{
	std::vector<std::pair<Point, int>> regions;

	for (const auto& point : points)
		regions.emplace_back(point, 1);

	for (int y = 0; y < map->height; ++y)
		for (int x = 0; x < map->width; ++x)
		{
			Point pos(x, y);
			int nearest = -1;
			int nearestDistance = INT_MAX;

			for (std::size_t i = 0; i < points.size(); ++i)
			{
				int distance = lengthSquared(points[i] - pos);

				if (distance < nearestDistance)
				{
					nearest = i;
					nearestDistance = distance;
				}
			}

			regions[nearest].first += pos;
			regions[nearest].second += 1;
		}

	for (std::size_t i = 0; i < points.size(); ++i)
		points[i] = regions[i].first / regions[i].second;
}

void MapGenerator::connectPoints(std::vector<Point>& points)
{
	std::vector<Point> connected;

	connected.emplace_back(points.back());
	points.pop_back();

	while (!points.empty())
	{
		Point bestFrom;
		int bestToIndex = -1;
		int bestDistance = INT_MAX;

		for (const auto& from : connected)
		{
			for (std::size_t i = 0; i < points.size(); ++i)
			{
				int distance = lengthSquared(points[i] - from);

				if (distance < bestDistance)
				{
					bestFrom = from;
					bestToIndex = i;
					bestDistance = distance;
				}
			}
		}

		Point to = points[bestToIndex];
		carveCorridor(bestFrom, to);
		connected.emplace_back(to);
		points.erase(points.begin() + bestToIndex);
	}
}

bool MapGenerator::canCarve(const Room& room) const
{
	for (int y = room.top - 1; y < room.top + room.height + 1; ++y)
		for (int x = room.left - 1; x < room.left + room.width + 1; ++x)
		{
			if (map->at(x, y).type != Tile::Wall)
				return false;
		}

	return true;
}

void MapGenerator::carveRoom(const Room& room, Tile::Type floor)
{
	for (int y = room.top; y < room.top + room.height; ++y)
		for (int x = room.left; x < room.left + room.width; ++x)
			map->at(x, y).type = floor;
}

void MapGenerator::carveCorridor(const Point& from, const Point& to)
{
	Point delta = to - from;
	Point primaryIncrement(sign(delta.x), 0);
	Point secondaryIncrement(0, sign(delta.y));
	int primary = std::abs(delta.x);
	int secondary = std::abs(delta.y);

	if (rng->getBool())
	{
		std::swap(primary, secondary);
		std::swap(primaryIncrement, secondaryIncrement);
	}

	std::vector<Point> line;
	Point current = from;
	int windingPoint = -1;

	if (primary > 1) // && rng->getBool())
		windingPoint = even(rng->getInt(primary));

	while (true)
	{
		line.emplace_back(current);

		if (primary > 0 && (primary != windingPoint || secondary == 0))
		{
			current += primaryIncrement;
			primary -= 1;
		}

		else if (secondary > 0)
		{
			current += secondaryIncrement;
			secondary -= 1;
		}

		else
		{
			assert(current == to);
			break;
		}
	}

	std::vector<Point> corridorTiles;

	for (std::size_t i = 1; i < line.size() - 1; ++i)
	{
		if (map->at(line[i]).type != Tile::Floor)
		{
			map->at(line[i]).type = Tile::Corridor;
			corridorTiles.emplace_back(line[i]);
		}
	}

	// Place doors
	map->at(corridorTiles.front()).type = Tile::Door;
	map->at(corridorTiles.back()).type = Tile::Door;
}

void MapGenerator::growMaze(int x, int y, int windingProb, Tile::Type floor)
{
	auto canCarve = [&] (const Point& pos, const Direction& dir)
	{
		if (!map->isInBounds(pos + dir * 3))
			return false;

		auto left = pos + dir + dir.left45();
		auto right = pos + dir + dir.right45();

		if (map->at(left).type != Tile::Wall || map->at(right).type != Tile::Wall)
			return false;

		left += dir;
		right += dir;

		if (map->at(left).type != Tile::Wall || map->at(right).type != Tile::Wall)
			return false;

		return map->at(pos + dir * 2).type == Tile::Wall;
	};

	for (int dy = -1; dy <= 1; ++dy)
		for (int dx = -1; dx <= 1; ++dx)
		{
			if (map->at(x + dx, y + dy).type != Tile::Wall)
				return;
		}

	map->at(x, y).type = floor;

	std::vector<Point> cells;
	Direction lastDir;

	cells.emplace_back(x, y);
	// maze.emplace_back(x, y);

	while (!cells.empty())
	{
		Point cell = cells.back();
		std::vector<Direction> unmadeCells;

		for (const auto& dir : Direction::Cardinal)
		{
			if (canCarve(cell, dir))
				unmadeCells.emplace_back(dir);
		}

		if (!unmadeCells.empty())
		{
			auto found = std::find(unmadeCells.begin(), unmadeCells.end(), lastDir);

			if (found == unmadeCells.end() || rng->getInt(100) < windingProb)
				lastDir = rng->getOne(unmadeCells);

			map->at(cell + lastDir).type = floor;
			map->at(cell + lastDir * 2).type = floor;

			// maze.emplace_back(cell + lastDir);
			// maze.emplace_back(cell + lastDir * 2);

			cells.emplace_back(cell + lastDir * 2);
		}

		else
		{
			cells.pop_back();
			lastDir = Direction::None;
		}
	}
}

void MapGenerator::removeWalls()
{
	for (int y = 0; y < map->height; ++y)
		for (int x = 0; x < map->width; ++x)
		{
			if (map->at(x, y).type != Tile::Wall)
				continue;

			bool removeWall = true;

			for (const auto& dir : Direction::All)
			{
				if (map->isInBounds(x + dir.x, y + dir.y) &&
					map->at(x + dir.x, y + dir.y).type != Tile::Wall &&
					// map->at(x + dir.x, y + dir.y).type != Tile::Corridor &&
					// map->at(x + dir.x, y + dir.y).type != Tile::Door &&
					map->at(x + dir.x, y + dir.y).type != Tile::Unused)
				{
					removeWall = false;
					break;
				}
			}

			if (removeWall)
				map->at(x, y).type = Tile::Unused;
		}
}
