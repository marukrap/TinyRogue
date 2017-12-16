#pragma once

// Credit: Thomas Jahn
// http://blog.pixelpracht.net/?p=340

#include "LightGridData.hpp"

#include <vector>

class Map;
class Console;

class Fov
{
public:
	explicit Fov(Console& console);

	void load(Map& map);
	void compute(const sf::Vector2f& pos, float range);

	void draw();

private:
	LightGridData* fetch(int x, int y);
	bool isInFov(int x, int y);

	void clearGrid();

	float getIntensity(int x, int y) const;
	void addShadowLightRecursive(int minx, int maxx, int miny, int maxy);

	void lightWall(int x, int y);
	void lightWalls(int x0, int y0, int x1, int y1, int dx, int dy);

private:
	Console& console;
	sf::Vector2f tileSize;

	Map* map = nullptr;
	sf::Vector2f position;
	float range;

	std::vector<LightGridData> grid;
	bool postproc = true; // light walls
	bool circle = false; // avoid ellipse shape
};
