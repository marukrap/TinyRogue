#pragma once

#include "Sector.hpp"

#include <SFML/Graphics/Color.hpp>

class LightGridData
{
public:
	void addScaled(const sf::Color& c, float intensity);
	void addAttenuation(const sf::Color& c, float intensity);
	void setAttenuation(const sf::Color& c, float intensity);

	void resetOcclusion();
	void calculateOcclusion(sf::Vector2f pos, bool normalize);
	void calculateOcclusionRecursive(sf::Vector2f pos, bool normalize);
	void calculateWallOcclusionRecursive(sf::Vector2f pos, bool normalize);

public:
	static const float UnknownOcclusion;

	int x = 0, y = 0; // spatial data
	bool opaque = false;
	float occlusion = UnknownOcclusion;
	sf::Color color;

	LightGridData* left = nullptr;
	LightGridData* right = nullptr;
	LightGridData* up = nullptr;
	LightGridData* down = nullptr;

private:
	Sector lightSector;
	Sector incomingA;
	Sector incomingB;
};
