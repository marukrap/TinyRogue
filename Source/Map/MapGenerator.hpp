#pragma once

#include "Tile.hpp"

#include <SFML/Graphics/Rect.hpp>

#include <vector>

class Level;
class Rng;

class MapGenerator
{
public:
	using Room = sf::IntRect;
	using Point = sf::Vector2i;

public:
	virtual ~MapGenerator() = default;

	void generate(Level& level, Rng& rng, int depth);

protected:
	void fill(Tile::Type tileType);

	void relaxation(std::vector<Point>& points);
	void connectPoints(std::vector<Point>& points);

	bool canCarve(const Room& room) const;
	void carveRoom(const Room& room, Tile::Type floor);
	void carveCorridor(const Point& from, const Point& to);

	void growMaze(int x, int y, int windingProb, Tile::Type floor);

	void removeWalls();

private:
	virtual void onGenerate() = 0;

protected:
	Level* map = nullptr;
	Rng* rng = nullptr;
	int depth = 0;
};
