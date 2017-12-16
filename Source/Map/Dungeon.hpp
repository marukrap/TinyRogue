#pragma once

#include "MapGenerator.hpp"

class Dungeon : public MapGenerator
{
private:
	void onGenerate() override;

	sf::Vector2i findFloor(const Room& room);

	void addTreasures(const Room& room);

	void placeItems();
	void placeTraps();
};
