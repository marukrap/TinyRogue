#include "LightGridData.hpp"

#include <algorithm> // min, max
#include <cmath> // ceil, abs

const float LightGridData::UnknownOcclusion = -1.f;

void LightGridData::addScaled(const sf::Color& c, float intensity)
{
	color.r = static_cast<sf::Uint8>(std::min(255, static_cast<int>(color.r + c.r * intensity)));
	color.g = static_cast<sf::Uint8>(std::min(255, static_cast<int>(color.g + c.g * intensity)));
	color.b = static_cast<sf::Uint8>(std::min(255, static_cast<int>(color.b + c.b * intensity)));
}

void LightGridData::addAttenuation(const sf::Color& c, float intensity)
{
	color.a = static_cast<sf::Uint8>(std::max(0, static_cast<int>(color.a - c.a * intensity)));
}

void LightGridData::setAttenuation(const sf::Color& c, float intensity)
{
	color.a = static_cast<sf::Uint8>(std::max(0, static_cast<int>(255 - c.a * intensity)));
}

void LightGridData::resetOcclusion()
{
	occlusion = UnknownOcclusion;
}

// evaluate this cells occlusion (ITERATOR version)
void LightGridData::calculateOcclusion(sf::Vector2f pos, bool normalize)
{
	if (opaque) // this cell itself occludes all light
	{
		lightSector.clear();
		occlusion = 1.f;
		return;
	}

	// vector (dx, dy) describes relative position from center
	int dx = static_cast<int>(std::ceil(pos.x)) - x - 1;
	int dy = static_cast<int>(std::ceil(pos.y)) - y - 1;

	// sign of vector components
	int sx = (dx == 0) ? 0 : (dx < 0) ? -1 : 1;
	int sy = (dy == 0) ? 0 : (dy < 0) ? -1 : 1;

	// offset necessary for vertical/horizontal lines
	int ox = (sy == 0) ? sx : 0;
	int oy = (sx == 0) ? sy : 0;

	// calculate the points that define the sector
	float x1 = x + 0.5f * (1 - sy + ox);
	float y1 = y + 0.5f * (1 + sx + oy);
	float x2 = x + 0.5f * (1 + sy + ox);
	float y2 = y + 0.5f * (1 - sx + oy);

	// calculate sector
	lightSector.setFromCoords(pos.x, pos.y, x1, y1, x2, y2, normalize);

	if (std::abs(dx) + std::abs(dy) > 1) // no direct connection with light source - there might be occlusion
	{
		// gather horizontal
		if (dx > 0 && right)
			incomingA.setIntersection(lightSector, right->lightSector);
		else if (dx < 0 && left)
			incomingA.setIntersection(lightSector, left->lightSector);
		else
			incomingA.clear();

		// gather vertical
		if (dy > 0 && down)
			incomingB.setIntersection(lightSector, down->lightSector);
		else if (dy < 0 && up)
			incomingB.setIntersection(lightSector, up->lightSector);
		else
			incomingB.clear();

		// combine exposure from both edges and compare with max possible exposure (myTheta)
		float myTheta = lightSector.theta;
		lightSector.setUnion(incomingA, incomingB);
		occlusion = 1.f - (lightSector.theta / myTheta);
	}

	else
		occlusion = 0.f; // ends recursion
}

// evaluate this cells occlusion (RECURSIVE version)
void LightGridData::calculateOcclusionRecursive(sf::Vector2f pos, bool normalize)
{
	if (occlusion != UnknownOcclusion)
		return;

	// offset relative to the center position
	int dx = static_cast<int>(std::ceil(pos.x)) - x - 1;
	int dy = static_cast<int>(std::ceil(pos.y)) - y - 1;

	// make sure relevant neighbours are evaluated first
	LightGridData* a = nullptr;

	if (dx > 0)
		a = right;
	else if (dx < 0)
		a = left;

	if (a)
		a->calculateOcclusionRecursive(pos, normalize);

	LightGridData* b = nullptr;

	if (dy > 0)
		b = down;
	else if (dy < 0)
		b = up;

	if (b)
		b->calculateOcclusionRecursive(pos, normalize);

	// the rest is similar to the iterator version...

	if (opaque)
	{
		lightSector.clear();
		occlusion = 1.f;
		return;
	}

	int sx = (dx == 0) ? 0 : (dx < 0) ? -1 : 1;
	int sy = (dy == 0) ? 0 : (dy < 0) ? -1 : 1;
	int ox = (sy == 0) ? sx : 0;
	int oy = (sx == 0) ? sy : 0;

	float x1 = x + 0.5f * (1 - sy + ox);
	float y1 = y + 0.5f * (1 + sx + oy);
	float x2 = x + 0.5f * (1 + sy + ox);
	float y2 = y + 0.5f * (1 - sx + oy);

	lightSector.setFromCoords(pos.x, pos.y, x1, y1, x2, y2, normalize);

	if (std::abs(dx) + std::abs(dy) > 1) // no direct connection with light source - there might be occlusion
	{
		// evaluate right
		if (a)
			incomingA.setIntersection(lightSector, a->lightSector);
		else
			incomingA.clear();

		// evaluate down
		if (b)
			incomingB.setIntersection(lightSector, b->lightSector);
		else
			incomingB.clear();

		// combine exposure from both edges and compare with max possible exposure (myTheta)
		float myTheta = lightSector.theta;
		lightSector.setUnion(incomingA, incomingB);
		occlusion = 1.f - (lightSector.theta / myTheta);
	}

	else
		occlusion = 0.f; // end recursion
}

void LightGridData::calculateWallOcclusionRecursive(sf::Vector2f pos, bool normalize)
{
	// offset relative to the center position
	int dx = static_cast<int>(std::ceil(pos.x)) - x - 1;
	int dy = static_cast<int>(std::ceil(pos.y)) - y - 1;

	// make sure relevant neighbours are evaluated first
	LightGridData* a = nullptr;

	if (dx > 0)
		a = right;
	else if (dx < 0)
		a = left;

	if (a)
		a->calculateOcclusionRecursive(pos, normalize);

	LightGridData* b = nullptr;

	if (dy > 0)
		b = down;
	else if (dy < 0)
		b = up;

	if (b)
		b->calculateOcclusionRecursive(pos, normalize);

	// the rest is similar to the iterator version...

	if (opaque)
	{
		lightSector.clear();
		occlusion = 1.f;
		return;
	}

	int sx = (dx == 0) ? 0 : (dx < 0) ? -1 : 1;
	int sy = (dy == 0) ? 0 : (dy < 0) ? -1 : 1;
	int ox = (sy == 0) ? sx : 0;
	int oy = (sx == 0) ? sy : 0;

	float x1 = x + 0.5f * (1 - sy + ox);
	float y1 = y + 0.5f * (1 + sx + oy);
	float x2 = x + 0.5f * (1 + sy + ox);
	float y2 = y + 0.5f * (1 - sx + oy);

	lightSector.setFromCoords(pos.x, pos.y, x1, y1, x2, y2, normalize);

	if (std::abs(dx) + std::abs(dy) > 1) // no direct connection with light source - there might be occlusion
	{
		// evaluate right
		if (a)
			incomingA.setIntersection(lightSector, a->lightSector);
		else
			incomingA.clear();

		// evaluate down
		if (b)
			incomingB.setIntersection(lightSector, b->lightSector);
		else
			incomingB.clear();

		// combine exposure from both edges and compare with max possible exposure (myTheta)
		float myTheta = lightSector.theta;
		lightSector.setUnion(incomingA, incomingB);
		occlusion = 1.f - (lightSector.theta / myTheta);
	}

	else
		occlusion = 0.f; // end recursion
}
