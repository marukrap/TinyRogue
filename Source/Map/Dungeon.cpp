#include "Dungeon.hpp"
#include "../World/Level.hpp"
#include "../Utility/Rng.hpp"

#include <algorithm> // min, max

namespace
{
	int odd(int number)
	{
		return number / 2 * 2 + 1;
	}

	int noFood = 0;
}

void Dungeon::onGenerate()
{
	fill(Tile::Wall);

	std::vector<sf::Vector2i> points;

	for (int i = 0; i < 9; ++i)
	{
		int x = rng->getInt(map->width);
		int y = rng->getInt(map->height);

		points.emplace_back(x, y);
	}

	for (int i = 0; i < 5; ++i)
		relaxation(points);

	// TODO: Gone rooms, secret doors/paths

	for (auto it = points.begin(); it != points.end(); )
	{
		auto& point = *it;

		Room room;
		room.width = odd(rng->rollDice(4, 3));
		room.height = odd(rng->rollDice(3, 3));
		room.left = odd(std::min(std::max(1, point.x - room.width / 2), map->width - room.width - 2));
		room.top = odd(std::min(std::max(1, point.y - room.height / 2), map->height - room.height - 2));

		point.x = odd(room.left + room.width / 2);
		point.y = odd(room.top + room.height / 2);
		// point.x = odd(room.left + rng->getInt(room.width));
		// point.y = odd(room.top + rng->getInt(room.height));

		if (canCarve(room))
		{
			map->rooms.emplace_back(room);
			carveRoom(room, Tile::Floor);
			++it;
		}

		else
			it = points.erase(it);
	}

	connectPoints(points);
	removeWalls();

	// Set room type
	// for (const auto& room : rooms)
	for (auto it = map->rooms.begin(); it != map->rooms.end(); )
	{
		const auto& room = *it;

		if (rng->getInt(10) < depth - 1)
		{
			if (rng->getInt(15) == 0) // maze room
			{
				carveRoom(room, Tile::Wall);

				for (int y = room.top; y < room.top + room.height; y += 2)
					for (int x = room.left; x < room.left + room.width; x += 2)
						growMaze(x, y, 35, Tile::Corridor);

				for (int y = room.top - 1; y < room.top + room.height + 1; ++y)
					for (int x = room.left - 1; x < room.left + room.width + 1; ++x)
					{
						if (map->at(x, y).type == Tile::Door)
							map->at(x, y).type = Tile::Corridor;
					}

				it = map->rooms.erase(it);
			}

			// else // dark room
			else if (rng->getInt(5) > 0) // HACK: make dark rooms less
			{
				carveRoom(room, Tile::Corridor);
				++it;
			}
		}

		else
			++it;
	}

	// Place stairs
	auto upStairs = findFloor(map->rooms.front());
	map->setUpStairs(upStairs);

	auto downStairs = findFloor(map->rooms.back());
	map->setDownStairs(downStairs);

	for (const auto& room : map->rooms)
	{
		int value = 0;

		// Put the gold in
		if (rng->getBool()) // && (!amulet || depth >= max_depth)
		{
			value = rng->getInt(50 + 10 * depth) + 2; // GOLDCALC

			auto gold = Item::createGold(value);
			gold->setPosition(findFloor(room));

			map->attach(std::move(gold));
		}

		// Put the monster in
		if (rng->getInt(100) < (value > 0 ? 80 : 25))
		{
			sf::Vector2i pos;

			do
				pos = findFloor(room);
			while (map->actorAt(pos));

			auto monster = Actor::createMonster(*rng, depth, false);
			monster->setPosition(pos);
			map->attach(std::move(monster));

			// TODO: give_pack:
		}
	}

	++noFood;

	// if (amulet && depth < max_depth)
		// return;

	placeItems();
	placeTraps();

	if (depth == 1)
		map->at(upStairs).type = Tile::Floor;
}

sf::Vector2i Dungeon::findFloor(const Room& room)
{
	sf::Vector2i pos;

	do
	{
		pos.x = room.left + rng->getInt(room.width);
		pos.y = room.top + rng->getInt(room.height);
	
	} while (map->at(pos).type != Tile::Floor && map->at(pos).type != Tile::Corridor);

	return pos;
}

void Dungeon::addTreasures(const Room& room)
{
	/* new_level.c */
	// treas_room: Add a treasure room

	const int minTreasures = 2;
	const int maxTreasures = 8;

	int spots = (room.width - 2) * (room.height - 2) - minTreasures;

	if (spots > maxTreasures - minTreasures)
		spots = maxTreasures - minTreasures;

	int numItems = rng->getInt(spots) + minTreasures;

	for (int i = 0; i < numItems; ++i)
	{
		sf::Vector2i pos;

		do
			pos = findFloor(room);
		while (map->itemAt(pos));

		auto item = Item::createItem(*rng, noFood);
		item->setPosition(pos);
		map->attach(std::move(item));
	}

	int numMonsters = rng->getInt(spots) + minTreasures;

	if (numMonsters < numItems + 2)
		numMonsters = numItems + 2;

	spots = (room.width - 2) * (room.height - 2);

	if (numMonsters > spots)
		numMonsters = spots;

	// Fill up room with monsters from the next level down
	for (int i = 0; i < numMonsters; ++i)
	{
		// spots = 0; // ?

		sf::Vector2i pos;

		do
			pos = findFloor(room);
		while (map->actorAt(pos));

		auto monster = Actor::createMonster(*rng, depth + 1, false);
		monster->setPosition(pos);
		monster->addFlag(Actor::IsMean);
		map->attach(std::move(monster));
	}
}

void Dungeon::placeItems()
{
	const int TreasureRoom = 15; // TREAS_ROOM 20 > 15
	const int MaxItems = 10; // MAXOBJ 9 > 10

	if (rng->getInt(TreasureRoom) == 0)
		addTreasures(rng->getOne(map->rooms));

	for (int i = 0; i < MaxItems; ++i)
	{
		if (rng->getInt(100) < 36)
		{
			sf::Vector2i pos;

			do
			{
				const auto& room = rng->getOne(map->rooms);
				pos = findFloor(room);

			} while (map->itemAt(pos));

			auto item = Item::createItem(*rng, noFood);
			item->setPosition(pos);
			map->attach(std::move(item));
		}
	}
}

void Dungeon::placeTraps()
{
	const int MaxTraps = 10;

	if (rng->getInt(10) < depth)
	{
		// int numTraps = rng->getInt(depth / 4) + 1;
		int numTraps = 1;

		if (depth >= 4)
			numTraps += rng->getInt(depth / 4);

		if (numTraps > MaxTraps)
			numTraps = MaxTraps;

		for (int i = 0; i < numTraps; ++i)
		{
			sf::Vector2i pos;

			do
			{
				const auto& room = rng->getOne(map->rooms);
				pos = findFloor(room);

			} while (map->trapAt(pos));

			auto type = static_cast<Trap::Type>(rng->getInt(Trap::TrapCount));
			auto trap = std::make_unique<Trap>(type);
			trap->setPosition(pos);
			map->attach(std::move(trap));
		}
	}
}
