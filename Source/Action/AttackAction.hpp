#pragma once

#include "Action.hpp"
#include "../Utility/Direction.hpp"

class AttackAction : public Action
{
public:
	AttackAction(Actor& target, const Direction& dir, Item* weapon = nullptr);

	bool perform(Actor& actor) override;

	static bool swing(int level, int armor, int wplus);

private:
	Actor& target;
	Direction dir;
	Item* weapon;
	bool thrown;
};
