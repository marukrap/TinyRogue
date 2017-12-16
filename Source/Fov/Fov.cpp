#include "Fov.hpp"
#include "../Map/Map.hpp"
#include "../Game/Console.hpp"

#include <SFML/Graphics/RenderTarget.hpp>

#include <cmath> // ceil

namespace
{
	const int FovAlpha = 225; // 230 > 225
}

Fov::Fov(Console& console)
	: console(console)
	, tileSize(console.getTileSize())
{
}

void Fov::load(Map& map)
{
	this->map = &map;

	// TODO: 맵의 크기가 변하지 않으면 opaque만 갱신하도록 변경

	grid.resize(map.width * map.height);

	for (int y = 0; y < map.height; ++y)
		for (int x = 0; x < map.width; ++x)
		{
			LightGridData* ld = fetch(x, y);

			ld->x = x;
			ld->y = y;

			ld->opaque = !map.at(x, y).transparent;

			ld->left = fetch(x - 1, y);
			ld->right = fetch(x + 1, y);
			ld->up = fetch(x, y - 1);
			ld->down = fetch(x, y + 1);
		}
}

void Fov::compute(const sf::Vector2f& pos, float range)
{
	clearGrid();

	this->position.x = pos.x + 0.5f; // (pos.x + tileSize.x / 2.f) / tileSize.x;
	this->position.y = pos.y + 0.5f; // (pos.y + tileSize.y / 2.f) / tileSize.y;
	this->range = range;

	int x = static_cast<int>(position.x);
	int y = static_cast<int>(position.y);

	if (map->isInBounds(x, y))
	{
		// HACK: used for Blind effect
		if (range < 1.f)
		{
			fetch(x, y)->color.a = 0;
			map->at(x, y).visible = true;
			map->at(x, y).explored = true;

			return;
		}

		float rangex = circle ? range * (tileSize.y / tileSize.x) : range;

		int minx = std::max(static_cast<int>(position.x - rangex), 0);
		int maxx = std::min(static_cast<int>(position.x + rangex), map->width - 1);
		int miny = std::max(static_cast<int>(position.y - range), 0);
		int maxy = std::min(static_cast<int>(position.y + range), map->height - 1);

		addShadowLightRecursive(minx, maxx, miny, maxy);

		if (postproc)
		{
			for (int dy = -1; dy <= 1; ++dy)
				for (int dx = -1; dx <= 1; ++dx)
				{
					int px = x + dx;
					int py = y + dy;

					if (map->isInBounds(px, py))
					{
						fetch(px, py)->setAttenuation(sf::Color::Black, getIntensity(px, py));
						map->at(px, py).visible = true;
						map->at(px, py).explored = true;
					}

					// TODO: 나중에 다시 확인해볼 것
					// else if (map->at(x, y).explored)
						// fetch(x, y)->color = sf::Color(0, 0, 0, FovAlpha);
				}

			lightWalls(minx, miny, x, y, -1, -1);
			lightWalls(x, miny, maxx, y, 1, -1);
			lightWalls(minx, y, x, maxy, -1, 1);
			lightWalls(x, y, maxx, maxy, 1, 1);
		}
	}
}

void Fov::draw()
{
	for (int y = 0; y < map->height; ++y)
		for (int x = 0; x < map->width; ++x)
		{
			sf::Uint8 alpha = 255 - fetch(x, y)->color.a;
			console.setColorA(x, y, alpha);
		}
}

LightGridData* Fov::fetch(int x, int y)
{
	if (x >= 0 && x < map->width && y >= 0 && y < map->height)
		return &grid[x + y * map->width];

	return nullptr;
}

bool Fov::isInFov(int x, int y)
{
	return fetch(x, y)->color.a < FovAlpha;
}

void Fov::clearGrid()
{
	for (int y = 0; y < map->height; ++y)
		for (int x = 0; x < map->width; ++x)
		{
			map->at(x, y).visible = false;

			if (map->at(x, y).explored)
				fetch(x, y)->color = sf::Color(0, 0, 0, FovAlpha);
			else
				fetch(x, y)->color = sf::Color::Black;
		}
}

float Fov::getIntensity(int x, int y) const
{
	// intensity is calculated based on distance from light source
	float dx = position.x - (x + 0.5f);
	float dy = position.y - (y + 0.5f);

	// convert the fov shape from an ellipse to a circle when using asymmetrical tileset
	if (circle)
		dx /= tileSize.y / tileSize.x;

	float dSqr = dx * dx + dy * dy;
	float rSqr = range * range;

	return std::max(0.f, 1.f - dSqr / rSqr);
}

void Fov::addShadowLightRecursive(int minx, int maxx, int miny, int maxy)
{
	// this is the recursive variant of the algorithm
	int centerX = std::max(0, static_cast<int>(std::ceil(position.x)) - 1);
	int centerY = std::max(0, static_cast<int>(std::ceil(position.y)) - 1);

	// reset occlusion
	for (int y = miny; y <= maxy; ++y)
		for (int x = minx; x <= maxx; ++x)
			fetch(x, y)->resetOcclusion();

	// calculate occlusion by starting from the edges, eventually visiting all cells in the relevant rect
	fetch(minx, miny)->calculateOcclusionRecursive(position, true);
	fetch(minx, maxy)->calculateOcclusionRecursive(position, true);

	// clear center (necessary HACK because left and right marching eval uses different Angle-normalization)
	for (int y = miny; y <= centerY; ++y)
		fetch(centerX, y)->resetOcclusion();

	fetch(maxx, miny)->calculateOcclusionRecursive(position, false);
	fetch(maxx, maxy)->calculateOcclusionRecursive(position, false);

	// shade based on occlusion & distance from light source
	for (int y = miny; y <= maxy; ++y)
		for (int x = minx; x <= maxx; ++x)
		{
			// cell receives light based on intensity and occlusion
			LightGridData* ld = fetch(x, y);
			ld->addAttenuation(sf::Color::Black, getIntensity(x, y) * (1.f - ld->occlusion));

			if (isInFov(x, y))
			{
				map->at(x, y).visible = true;
				map->at(x, y).explored = true;
			}

			else if (map->at(x, y).explored)
				ld->color = sf::Color(0, 0, 0, FovAlpha);
		}
}

void Fov::lightWall(int x, int y)
{
	LightGridData* ld = fetch(x, y);
	ld->setAttenuation(sf::Color::Black, getIntensity(x, y));

	if (isInFov(x, y))
	{
		map->at(x, y).visible = true;
		map->at(x, y).explored = true;
	}

	else
		ld->color = sf::Color(0, 0, 0, FovAlpha);

	// TODO: 나중에 다시 확인해볼 것
	// else if (map->at(x, y).explored)
		// ld->color = sf::Color(0, 0, 0, FovAlpha);
}

void Fov::lightWalls(int x0, int y0, int x1, int y1, int dx, int dy)
{
	for (int cy = y0; cy <= y1; ++cy)
		for (int cx = x0; cx <= x1; ++cx)
		{
			int x2 = cx + dx;
			int y2 = cy + dy;

			// fetch(cx, cy).color.a < 229 - fix graphical glitches when walking
			if (fetch(cx, cy)->color.a < FovAlpha - 1 && !fetch(cx, cy)->opaque)
			{
				if (x2 >= x0 && x2 <= x1 && fetch(x2, cy)->opaque)
					lightWall(x2, cy);
				if (y2 >= y0 && y2 <= y1 && fetch(cx, y2)->opaque)
					lightWall(cx, y2);
				if (x2 >= x0 && x2 <= x1 && y2 >= y0 && y2 <= y1 && fetch(x2, cy)->opaque && fetch(cx, y2)->opaque && fetch(x2, y2)->opaque)
					lightWall(x2, y2);
			}
		}
}
