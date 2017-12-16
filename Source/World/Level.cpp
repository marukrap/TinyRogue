#include "Level.hpp"
#include "../Graphics/BloomEffect.hpp"
#include "../Graphics/Color.hpp"
#include "../Utility/Rng.hpp"
#include "../Utility/Utility.hpp"

// REMOVE:
#include "World.hpp" 
#include "ActorPlayer.hpp"
#include "../Game/Console.hpp"
//

#include <algorithm> // find_if, remove_if
#include <functional> // mem_fn
#include <cassert>

namespace
{
	// Used for hallucination effect
	const std::vector<sf::Color> randomColors =
	{
		/* Color::Black, */ Color::DarkViolet, Color::DarkBlue, Color::DarkGray,
		Color::DarkBrown, Color::DarkGreen, Color::Red, Color::Gray,
		Color::Blue, Color::Brown, Color::LightGray, Color::Green,
		Color::Pink, Color::Cyan, Color::Yellow, Color::White
	};

	const sf::Time secondsPerTick = sf::seconds(0.5f);
}

Level::Level(Rng& rng, int width, int height)
	: Map(width, height)
	, rng(rng)
{
}

#include "../World/Items/Ring.hpp"

void Level::attach(Actor::Ptr actor)
{
	// REMOVE: static 함수인 createMonster 함수를 대신해서 이곳에서 임시로 처리
	if (actor->getType() == Actor::Enemy && getWorld().getPlayerActor().hasRing(Ring::Aggravate))
		actor->startRun();

	actors.emplace_back(std::move(actor));
}

Actor::Ptr Level::detach(Actor& actor)
{
	auto found = std::find_if(actors.begin(), actors.end(), [&] (const auto& a) { return a.get() == &actor; });
	assert(found != actors.end());

	auto result = std::move(*found);
	actors.erase(found);

	return result;
}

void Level::attach(Item::Ptr item)
{
	items.emplace_back(std::move(item));
}

Item::Ptr Level::detach(Item& item)
{
	auto found = std::find_if(items.begin(), items.end(), [&] (const auto& i) { return i.get() == &item; });
	assert(found != items.end());

	auto result = std::move(*found);
	items.erase(found);

	return result;
}

void Level::attach(Trap::Ptr trap)
{
	traps.emplace_back(std::move(trap));
}

Trap::Ptr Level::detach(Trap& trap)
{
	auto found = std::find_if(traps.begin(), traps.end(), [&] (const auto& t) { return t.get() == &trap; });
	assert(found != traps.end());

	auto result = std::move(*found);
	traps.erase(found);

	return result;
}

Actor* Level::actorAt(const sf::Vector2i& pos) const
{
	for (const auto& actor : actors)
	{
		if (actor->getPosition() == pos)
			return actor.get();
	}

	return nullptr;
}

Item* Level::itemAt(const sf::Vector2i& pos) const
{
	// NOTE: If there're more than 2 items, pick up the top
	for (auto it = items.rbegin(); it != items.rend(); ++it)
	{
		if ((*it)->getPosition() == pos)
			return (*it).get();
	}

	return nullptr;
}

void Level::foreachActors(std::function<void(Actor&)> function)
{
	for (auto& actor : actors)
		function(*actor);
}

void Level::foreachItems(std::function<void(Item&)> function)
{
	for (auto& item : items)
		function(*item);
}

Trap* Level::trapAt(const sf::Vector2i& pos) const
{
	for (const auto& trap : traps)
	{
		if (trap->getPosition() == pos)
			return trap.get();
	}

	return nullptr;
}

sf::Vector2i Level::findPassableTile()
{
	while (true)
	{
		int x = rng.getInt(width);
		int y = rng.getInt(height);

		if (at(x, y).passable)
			return sf::Vector2i(x, y);
	}
}

sf::Vector2i Level::findRandomDoor(const sf::Vector2i& pos)
{
	std::vector<sf::Vector2i> doors;

	for (const auto& room : rooms)
	{
		sf::IntRect rect(room.left - 1, room.top - 1, room.width + 2, room.height + 2);

		if (rect.contains(pos))
		{
			for (int y = rect.top; y < rect.top + rect.height; ++y)
				for (int x = rect.left; x < rect.left + rect.width; ++x)
				{
					if (at(x, y).type == Tile::Door)
						doors.emplace_back(x, y);
				}

			break;
		}
	}

	if (doors.empty())
		return sf::Vector2i(-1, -1);

	return doors[rng.getInt(doors.size())];
}

bool Level::isInRoom(const sf::Vector2i& pos) const
{
	for (const auto& room : rooms)
		if (room.contains(pos))
			return true;

	return false;
}

bool Level::lightRoom(const sf::Vector2i& pos)
{
	for (const auto& room : rooms)
		if (room.contains(pos))
		{
			for (int y = room.top; y < room.top + room.height; ++y)
				for (int x = room.left; x < room.left + room.width; ++x)
					at(x, y).transparent = true;

			// NOTE: You have to reload fov because Fov class cached the old map
			getWorld().reloadFov();

			return true;
		}

	return false;
}

void Level::setUpStairs(const sf::Vector2i& pos)
{
	at(pos).type = Tile::UpStairs;
	upStairs = pos;
}

void Level::setDownStairs(const sf::Vector2i& pos)
{
	at(pos).type = Tile::DownStairs;
	downStairs = pos;
}

void Level::removeDeadActors()
{
	auto it = std::remove_if(actors.begin(), actors.end(), std::mem_fn(&Actor::isMarkedForRemoval));
	actors.erase(it, actors.end());
}

void Level::colorize()
{
	hallucinate = true;
}

void Level::repaint()
{
	for (int y = 0; y < height; ++y)
		for (int x = 0; x < width; ++x)
		{
			Tile& tile = at(x, y);

			switch (tile.type)
			{
			case Tile::Wall:
				tile.color = Color::LightGray;
				break;

			case Tile::Floor:
			case Tile::Corridor:
				tile.color = Color::LightGray * sf::Color(100, 100, 100);
				break;

			case Tile::Door:
				tile.color = Color::Brown;
				break;

			case Tile::UpStairs:
				tile.color = Color::Cyan;
				break;
			}
		}

	for (auto& item : items)
		item->repaint();

	for (auto& actor : actors)
		actor->repaint();
}

void Level::update(sf::Time dt)
{
	for (auto& item : items)
		item->update(dt);

	for (auto& actor : actors)
		actor->update(dt);

	if (BloomEffect::hallucination)
	{
		elapsedTime += dt;

		if (elapsedTime >= secondsPerTick)
		{
			elapsedTime -= secondsPerTick;
			hallucinate = true;
		}
	}
}

void Level::draw(Console& console)
{
	if (hallucinate)
	{
		// TODO: Turn stairs to random item glyphs

		elapsedTime = sf::Time::Zero;
		hallucinate = false;

		for (int y = 0; y < height; ++y)
			for (int x = 0; x < width; ++x)
			{
				if (at(x, y).visible)
					at(x, y).color = randomColors[randomInt(randomColors.size())];
			}

		for (auto& item : items)
		{
			if (at(item->getPosition()).visible)
			{
				item->setChar(Item::getRandomChar());
				item->setColor(randomColors[randomInt(randomColors.size())]);
			}
		}

		for (auto& actor : actors)
		{
			if (at(actor->getPosition()).visible)
			{
				if (actor->getType() != Actor::Hero)
				{
					if (actor->getChar() == L'X' && actor->getDisguise() != L'X') // TODO: 버그 있음
						actor->setDisguise(Item::getRandomChar());
					else
						actor->setDisguise(randomInt(L'A', L'Z'));
				}

				actor->setColor(randomColors[randomInt(randomColors.size())]);
			}
		}
	}

	Map::draw(console);

	for (const auto& trap : traps)
	{
		if (at(trap->getPosition()).visible)
		{
			if (trap->isActive())
				trap->draw(console);
		}
	}

	for (const auto& item : items)
	{
		if (at(item->getPosition()).visible)
			item->draw(console);
	}

	for (const auto& actor : actors)
	{
		if (at(actor->getPosition()).visible)
		{
			if (getWorld().isVisible(*actor))
				actor->draw(console);
		}
	}
}

void Level::debugDraw(Console & console)
{
	Map::draw(console);

	for (const auto& trap : traps)
		trap->draw(console);

	for (const auto& item : items)
		item->draw(console);

	for (const auto& actor : actors)
		actor->draw(console);
}

// TODO: Better name
void Level::postProcess(Console& console)
{
	for (const auto& item : items)
	{
		if (!at(item->getPosition()).visible && item->hasFlag(Item::IsDetected))
			console.setChar(item->getPosition().x, item->getPosition().y, (item->getType() == ItemType::Food ? item->getChar() : '$'), sf::Color::White);
	}

	for (const auto& actor : actors)
	{
		if (!at(actor->getPosition()).visible && getWorld().getPlayerActor().hasFlag(Actor::SeeMonster))
			console.setChar(actor->getPosition().x, actor->getPosition().y, actor->getChar(), sf::Color::White);
	}
}
