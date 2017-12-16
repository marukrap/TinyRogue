#pragma once

#include "Action.hpp"
#include "../Utility/Direction.hpp"

class MonsterAction : public Action
{
public:
	bool perform(Actor& actor) override;

private:
	void wake(Actor& actor);

	bool move(Actor& actor);
	bool chase(Actor& actor);

	bool hasDestination(Actor& actor) const;

	bool moveToDestination(Actor& actor);
	bool moveForward(Actor& actor);
	bool moveBackward(Actor& actor);

	bool moveRandom(Actor& actor);

	Direction getNextDir(const sf::Vector2i& from, const sf::Vector2i& to) const;
};
