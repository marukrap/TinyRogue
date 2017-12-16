#include "WalkAction.hpp"
#include "ItemAction.hpp"
#include "AttackAction.hpp"
#include "../World/World.hpp"
#include "../World/Items/Scroll.hpp"
#include "../Graphics/Color.hpp"
#include "../Utility/Utility.hpp"

WalkAction::WalkAction(int dx, int dy)
	: dir(dx, dy)
{
}

WalkAction::WalkAction(const Direction& dir)
	: dir(dir)
{
}

bool WalkAction::perform(Actor& actor)
{
	// TODO: 대각선 이동과 공격을 허용할지 말지 

	if (dir.x == 0 && dir.y == 0)
	{
		actor.lastDir = dir;
		return true; // return std::make_unique<RestAction>()->perform(actor);
	}

	if (actor.hasFlag(Actor::NoMove))
	{
		if (actor.getType() == Actor::Hero)
			message(L"you are still stuck in the bear trap.", Color::White);

		actor.addEffect(Actor::NoMove, -1);
		return true;
	}

	Level& level = getLevel();
	sf::Vector2i pos = actor.getPosition();

	if (actor.hasFlag(Actor::IsHeld))
	{
		auto target = level.actorAt(pos + dir);

		if (target && target->getChar() == L'F')
			return std::make_unique<AttackAction>(*target, dir)->perform(actor);

		message(L"you are being held.", Color::White);
		return true;
	}

	// NOTE: Used for scare scrolls, but it may waste some performance
	auto passable = [&] (const sf::Vector2i& pos)
	{
		if (!level.at(pos).passable)
			return false;

		if (actor.getType() != Actor::Hero)
		{
			Item* item = level.itemAt(pos);

			if (item && item->isSame(ItemType::Scroll, Scroll::Scare))
				return false;
		}

		return true;
	};
	//

	if ((actor.hasFlag(Actor::IsConfused) && randomInt(5) != 0) ||
		(actor.getChar() == L'P' && randomInt(5) == 0) ||
		(actor.getChar() == L'B' && randomInt(2) == 0))
	{
		/* move.c */
		// rndmove: Move in a random direction if the monster/person is confused

		dir = Direction::All[randomInt(8)];

		if (randomInt(20) == 0)
			actor.removeFlag(Actor::IsConfused);

		// if (!level.at(pos + dir).passable)
		if (!passable(pos + dir))
			return true; // don't move
	}

	// else if (!level.at(pos + dir).passable)
	else if (!passable(pos + dir))
	{
		if (actor.getType() == Actor::Hero && actor.hasFlag(Actor::IsBlind))
		{
			// Bump into the wall
			if (level.at(pos + dir).type == Tile::Wall)
				level.at(pos + dir).explored = true;

			// Recompute fov
			getWorld().computeFov();

			return false;
		}

		int iterations = (actor.getType() == Actor::Hero ? 1 : 2);
		
		Direction oldDir = dir;
		Direction leftDir = dir;
		Direction rightDir = dir;

		for (int i = 0; i < iterations; ++i)
		{
			leftDir = leftDir.left45();
			rightDir = rightDir.right45();

			// if (level.at(pos + leftDir).passable && !level.actorAt(pos + leftDir))
			if (passable(pos + leftDir) && !level.actorAt(pos + leftDir))
			{
				dir = leftDir;
				break;
			}

			// if (level.at(pos + rightDir).passable && !level.actorAt(pos + rightDir))
			if (passable(pos + rightDir) && !level.actorAt(pos + rightDir))
			{
				dir = rightDir;
				break;
			}
		}

		if (oldDir == dir)
			return false;
	}

	auto target = level.actorAt(pos + dir);

	if (target)
		return std::make_unique<AttackAction>(*target, dir)->perform(actor);

	actor.move(dir);
	actor.lastDir = dir;

	// TODO: enter_room, door_open

	if (actor.getType() == Actor::Hero && !actor.hasFlag(Actor::IsLevitating))
	{
		auto item = level.itemAt(pos + dir);

		if (item)
		{
			if (item->hasFlag(Item::IsFound))
				message(L"{0} moved onto {1}.", { actor, *item });
			else
				std::make_unique<PickUpAction>(*item)->perform(actor); // Always returns true
		}

		auto trap = level.trapAt(pos + dir);

		if (trap)
			trap->beTrapped(actor);
	}

	return true;
}

RunAction::RunAction(int dx, int dy)
	: dir(dx, dy)
{
}

RunAction::RunAction(const Direction& dir)
	: dir(dir)
{
}

bool RunAction::perform(Actor& actor)
{
	// TODO: 체력이 전부 회복될 때까지 or 100턴 대기
	if (dir.x == 0 && dir.y == 0)
		return false;

	Level& level = getLevel();
	sf::Vector2i pos = actor.getPosition();

	if (!level.at(pos + dir).passable)
	{
		std::vector<Direction> passables;

		for (const auto& d : Direction::All)
		{
			if (d.x == -dir.x && d.y == -dir.y)
				continue;

			if (level.at(pos + d).passable)
				passables.emplace_back(d);
		}

		if (passables.size() != 1 || level.actorAt(pos + passables[0]))
			return false;

		dir = passables[0];
	}

	else
	{
		auto target = level.actorAt(pos + dir);

		if (target)
		{
			actor.lastDir = Direction::None;
			return std::make_unique<AttackAction>(*target, dir)->perform(actor);
		}
	}

	actor.move(dir);
	actor.lastDir = dir;
	pos += dir;

	auto item = level.itemAt(pos);

	if (item)
	{
		if (!actor.hasFlag(Actor::IsLevitating))
			std::make_unique<PickUpAction>(*item)->perform(actor);

		// Returns true but stop running
		actor.lastDir = Direction::None;
		return true;
	}

	if (!actor.hasFlag(Actor::IsLevitating))
	{
		auto trap = level.trapAt(pos);

		if (trap)
		{
			trap->beTrapped(actor);
				
			actor.lastDir = Direction::None;
			return true;
		}
	}

	std::vector<Direction> nextDirs =
	{
		dir,
		dir.left90(),
		dir.right90(),
		Direction::None,
	};

	// Doors or stairs
	for (const auto& next : nextDirs)
	{
		if (level.at(pos + next).type == Tile::Door ||
			level.at(pos + next).type == Tile::UpStairs ||
			level.at(pos + next).type == Tile::DownStairs)
		{
			actor.lastDir = Direction::None;
			return true;
		}
	}

	nextDirs.pop_back();
	nextDirs.emplace_back(dir.left45());
	nextDirs.emplace_back(dir.right45());

	// TODO: itemAt, trapAt 콜 최소화
	for (const auto& next : nextDirs)
	{
		if (level.itemAt(pos + next))
		{
			actor.lastDir = Direction::None;
			return true;
		}

		if (actor.hasFlag(Actor::IsBlind))
			continue;

		auto trap = level.trapAt(pos + next);

		if (trap && trap->isActive())
		{
			actor.lastDir = Direction::None;
			return true;
		}
	}

	// UNDONE: 통로에서 갈림길이 나온 경우 (임시)
	if (dir.x == 0 || dir.y == 0)
	{
		if (level.at(pos + dir).passable &&
			!level.at(pos - dir + dir.left90()).passable &&
			!level.at(pos - dir + dir.right90()).passable)
		{
			if (level.at(pos - dir + dir.left45()).passable ||
				level.at(pos - dir + dir.right45()).passable)
				actor.lastDir = Direction::None;
		}
	}
	//

	return true;
}

bool AscendAction::perform(Actor& actor)
{
	if (actor.levitCheck())
		return false;

	if (getLevel().at(actor.getPosition()).type == Tile::UpStairs)
	{
		getWorld().ascend();
		message(L"you ascend.", Color::White);

		return true;
	}

	message(L"i see no way up.", Color::White);

	return false;
}

bool DescendAction::perform(Actor& actor)
{
	if (actor.levitCheck())
		return false;

	if (getLevel().at(actor.getPosition()).type == Tile::DownStairs)
	{
		getWorld().descend();
		message(L"you descend.", Color::White);

		return true;
	}

	message(L"i see no way down.", Color::White);

	return false;
}

#include <iostream>

bool SearchAction::perform(Actor& actor)
{
	// TODO: Searching...
	std::cout << "Searching...\n";

	return true;
}

bool InteractAction::perform(Actor& actor)
{
	Tile::Type tile = getLevel().at(actor.getPosition()).type;

	if (tile == Tile::UpStairs)
		return std::make_unique<AscendAction>()->perform(actor);

	if (tile == Tile::DownStairs)
		return std::make_unique<DescendAction>()->perform(actor);

	Item* item = getLevel().itemAt(actor.getPosition());

	if (item)
		return std::make_unique<PickUpAction>(*item)->perform(actor);

	return std::make_unique<SearchAction>()->perform(actor);
}
