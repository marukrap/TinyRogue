#include "MonsterAction.hpp"
#include "WalkAction.hpp"
#include "../World/World.hpp"
#include "../World/ActorPlayer.hpp"
#include "../World/Items/Ring.hpp"
#include "../Utility/Utility.hpp"

#include <cmath> // abs
#include <algorithm> // swap
#include <iostream>

bool MonsterAction::perform(Actor& actor)
{
	/* chase.c */
	// runners: Make all the running monsters move

	// TODO: 시야(visible)를 이용하는 것이 아니라 방(room)을 기준으로 할 것
	//       시야를 기준으로 하는 경우 플레이어가 Blind 상태일 때 적도 플레이어를 보지 못함
	//       또한 wake를 몬스터가 행동할 때가 아닌 플레이어가 방에 입장하는 시점에 호출하도록 변경할 것
	// NOTE: enter_room > door_open > wake_monster

	if (getLevel().at(actor.getPosition()).visible)
		wake(actor);

	if (actor.hasFlag(Actor::IsHeld) || !actor.hasFlag(Actor::IsRunning))
		return true;

	if (!move(actor))
		return true;

	// TODO: player -> target
	const Actor& player = getWorld().getPlayerActor();

	if (actor.hasFlag(Actor::IsFly) && lengthSquared(player.getPosition() - actor.getPosition()) >= 3)
		move(actor);

	return true;
}

void MonsterAction::wake(Actor& actor)
{
	/* monsters.c */
	// wake_monster: What to do when the hero steps next to a monster

	Actor& player = getWorld().getPlayerActor();

	if (!player.hasRing(Ring::Stealth) && !player.hasFlag(Actor::IsLevitating) &&
		!actor.hasFlag(Actor::IsRunning) && !actor.hasFlag(Actor::IsHeld) &&
		actor.hasFlag(Actor::IsMean) && randomInt(3) > 0)
	{
		actor.addFlag(Actor::IsRunning);
		std::wcout << actor.getTheName() << " noticed you.\n";
	}

	if (actor.getChar() == L'M' && actor.hasFlag(Actor::IsRunning) &&
		!actor.hasFlag(Actor::IsFound) && !actor.hasFlag(Actor::IsCancelled) &&
		!player.hasFlag(Actor::IsBlind) && !player.hasFlag(Actor::IsHallucinating))
	{
		actor.addFlag(Actor::IsFound);

		if (!player.resistCheck(Actor::Magic))
		{
			player.addEffect(Actor::IsConfused, spread(20)); // HUHDURATION 20

			message(L"{0}'s gaze has confused you.", { actor });
			playSound(SoundID::Confusion);
		}

#ifdef _DEBUG
		else
			std::wcout << L"you resisted " << actor.getTheName() << L"'s gaze.\n";
#endif
	}

	if (actor.hasFlag(Actor::IsGreed) && !actor.hasFlag(Actor::IsRunning))
	{
		actor.addFlag(Actor::IsRunning);

		/*
		// TODO:
		// Let greedy ones guard gold
		if (proom->r_goldval)
			tp->t_dest = &proom->r_gold;
		else
			tp->t_dest = &hero;
		*/
	}
}

bool MonsterAction::move(Actor& actor)
{
	// move_monst: Execute a single turn of running for a monster

	if (!actor.hasFlag(Actor::IsSlowed) || actor.turn)
		if (!chase(actor))
			return false;

	if (actor.hasFlag(Actor::IsHasted))
		if (!chase(actor))
			return false;

	actor.turn ^= true;

	return true;
}

bool MonsterAction::chase(Actor& actor)
{
	// do_chase: Make one thing chase another

	// NOTE: This is not Rogue's algorithm

	// REMOVE:
	Level& level = getLevel();
	Actor& player = getWorld().getPlayerActor();

	const sf::Vector2i& pos = actor.getPosition();
	const Tile& tile = level.at(pos);
	//

	if (tile.visible)
	{
		actor.destination = player.getPosition();
		actor.turnsWaited = 0;

		return moveToDestination(actor);
	}

	if (actor.turnsWaited >= 6)
		return moveRandom(actor);

	else // <= 5
	{
		bool result = false;

		if (hasDestination(actor))
		{
			if (actor.destination == pos)
			{
				actor.destination = { -1, -1 };

				result = moveForward(actor);
			}

			else
				result = moveToDestination(actor);
		}

		else
		{
			if (level.isInRoom(pos))
			{
				actor.destination = level.findRandomDoor(pos);

				if (hasDestination(actor))
					result = moveToDestination(actor);
				else
					result = moveForward(actor);
			}

			else if (tile.type == Tile::Door)
			{
				actor.destination = level.findRandomDoor(pos);

				if (hasDestination(actor))
					result = moveForward(actor); // moveToDestination(actor);
				else
					result = moveBackward(actor);
			}

			else // Corridor
				result = moveForward(actor);
		}

		if (result)
			actor.turnsWaited = 0;
		else if (actor.turnsWaited == 5)
			return moveRandom(actor);
		else
			++actor.turnsWaited;

		return result;
	}

	std::cout << "MonsterAction::chase() - bug?\n";
	return false;
}

bool MonsterAction::hasDestination(Actor& actor) const
{
	return getLevel().isInBounds(actor.destination);
}

bool MonsterAction::moveToDestination(Actor& actor)
{
	auto dir = getNextDir(actor.getPosition(), actor.destination);

	return std::make_unique<WalkAction>(dir)->perform(actor);
}

bool MonsterAction::moveForward(Actor& actor)
{
	auto dir = actor.lastDir;

	if (dir == Direction::None)
		dir = Direction::All[randomInt(8)];

	return std::make_unique<WalkAction>(dir)->perform(actor);
}

bool MonsterAction::moveBackward(Actor& actor)
{
	actor.turnsWaited = 0;

	auto dir = actor.lastDir;

	if (dir == Direction::None)
		dir = Direction::All[randomInt(8)];
	else
		dir = -dir;

	return std::make_unique<WalkAction>(dir)->perform(actor);
}

bool MonsterAction::moveRandom(Actor& actor)
{
	actor.destination = { -1, -1 };

	if (getLevel().isInRoom(actor.getPosition()))
	{
		actor.lastDir = Direction::All[randomInt(8)];
		actor.turnsWaited = (actor.turnsWaited + 1) % 8;

		return moveForward(actor);
		
		/*
		actor.destination = level.findRandomDoor(pos);
		actor.turnsWaited = (actor.turnsWaited + 1) % 8;

		if (hasDestination(actor))
			return moveToDestination(actor);
		else
			return moveForward(actor);
		*/
	}

	else
	{
		// TODO: 이동 가능한 방향 중에서 선택?
		actor.lastDir = Direction::All[randomInt(8)];

		if (moveForward(actor))
		{
			actor.turnsWaited = 5;
			return true;
		}

		else
		{
			actor.turnsWaited = 6;
			return false;
		}
	}

	return false;
}

Direction MonsterAction::getNextDir(const sf::Vector2i& from, const sf::Vector2i& to) const
{
	sf::Vector2i delta = to - from;
	sf::Vector2i primaryIncrement(sign(delta.x), 0);
	sf::Vector2i secondaryIncrement(0, sign(delta.y));
	int primary = std::abs(delta.x);
	int secondary = std::abs(delta.y);

	if (secondary > primary)
	{
		std::swap(primary, secondary);
		std::swap(primaryIncrement, secondaryIncrement);
	}

	Direction dir;
	int error = 0;

	dir += primaryIncrement;
	error += secondary;

	if (error * 2 >= primary)
	{
		dir += secondaryIncrement;
		error -= primary;
	}

	return dir;
}
