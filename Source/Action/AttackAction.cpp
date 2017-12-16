#include "AttackAction.hpp"
#include "WalkAction.hpp"
#include "../World/World.hpp"
#include "../World/Items/Potion.hpp" // pickColor
#include "../World/Items/Ring.hpp"
#include "../Graphics/Color.hpp"
#include "../Utility/Utility.hpp"

#include <sstream>
#include <iostream>

namespace
{
	/* fight.c */
	// Strings for hitting
	const std::vector<std::wstring> hitStrings =
	{
		L" scored an excellent hit on ",
		L" hit ",
		L" have injured ",
		L" swing and hit ",
		L" scored an excellent hit on ",
		L" hit ",
		L" has injured ",
		L" swings and hits "
	};

	// Strings for missing
	const std::vector<std::wstring> missStrings =
	{
		L" miss",
		L" swing and miss",
		L" barely miss",
		L" don't hit",
		L" misses",
		L" swings and misses",
		L" barely misses",
		L" doesn't hit",
	};

	// Adjustments to hit probabilities due to strength
	const std::vector<int> hitBonus =
	{
		-7, -6, -5, -4, -3, -2, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		 0,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  3,
	};

	// Adjustments to damage done due to strength
	const std::vector<int> damageBonus =
	{
		-7, -6, -5, -4, -3, -2, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		 1,  1,  2,  3,  3,  4,  5,  5,  5,  5,  5,  5,  5,  5,  5,  6,
	};
}

AttackAction::AttackAction(Actor& target, const Direction& dir, Item* weapon)
	: target(target)
	, dir(dir)
	, weapon(weapon)
	, thrown(weapon ? true : false)
{
}

bool AttackAction::perform(Actor& actor)
{
	/* fight.c */
	// fight: The player attacks the monster
	// attack: The monster attacks the player

	// The player attacks a xeroc
	if (target.getChar() == L'X' && target.getDisguise() != L'X' && !actor.hasFlag(Actor::IsBlind))
	{
		target.setDisguise(L'X');

		// TODO: if (actor.hasFlag(Actor::IsHallucinating)) ...

		message(actor.chooseStr(L"heavy!  That's a nasty critter!",
								L"wait!  That's a xeroc!"), Color::Red);

		if (!thrown)
			return true;
	}

	// A xeroc attacks the player
	else if (actor.getChar() == L'X' && actor.getDisguise() != L'X' && !target.hasFlag(Actor::IsBlind))
	{
		actor.setDisguise(L'X');

		// TODO: if (target.hasFlag(Actor::IsHallucinating)) ...
	}

	// REMOVE: Move to WalkAction
	if (!actor.hasFlag(Actor::IsConfused))
	{
		if ((actor.getType() == target.getType()) ||
			(actor.getType() == Actor::Ally && target.getType() == Actor::Hero))
		{
			Level& level = getLevel();
			sf::Vector2i pos = actor.getPosition();

			Direction leftDir = dir;
			Direction rightDir = dir;

			for (int i = 0; i < 2; ++i)
			{
				leftDir = leftDir.left45();
				rightDir = rightDir.right45();

				if (level.at(pos + leftDir).passable && !level.actorAt(pos + leftDir))
					return std::make_unique<WalkAction>(leftDir)->perform(actor);

				if (level.at(pos + rightDir).passable && !level.actorAt(pos + rightDir))
					return std::make_unique<WalkAction>(rightDir)->perform(actor);
			}

			return false;
		}

		// Swap positions?
		else if (actor.getType() == Actor::Hero && target.getType() == Actor::Ally)
			return false;
	}
	//

	// roll_em: Roll several attacks
	std::string damageStr = actor.getDamage();
	int hitPlus = 0;
	int damagePlus = 0;

	if (!weapon && !thrown)
		weapon = actor.getEquipment(Actor::Weapon);

	if (weapon)
	{
		hitPlus = weapon->getHitPlus();
		damagePlus = weapon->getDamagePlus();

		if (actor.isEquipment(Actor::LeftRing, Ring::AddHit))
			hitPlus += actor.getEquipment(Actor::LeftRing)->getRingPower();
		else if (actor.isEquipment(Actor::LeftRing, Ring::AddDamage))
			damagePlus += actor.getEquipment(Actor::LeftRing)->getRingPower();

		if (actor.isEquipment(Actor::RightRing, Ring::AddHit))
			hitPlus += actor.getEquipment(Actor::RightRing)->getRingPower();
		else if (actor.isEquipment(Actor::RightRing, Ring::AddDamage))
			damagePlus += actor.getEquipment(Actor::RightRing)->getRingPower();

		damageStr = weapon->getDamage();
		
		if (thrown)
		{
			Item* currentWeapon = actor.getEquipment(Actor::Weapon);

			if (weapon->hasFlag(Item::IsMissile) && currentWeapon &&
				weapon->getLaunch() == currentWeapon->getWhich())
			{
				damageStr = weapon->getDamageThrown();
				hitPlus += currentWeapon->getHitPlus();
				damagePlus += currentWeapon->getDamagePlus();
			}

			else if (weapon->getLaunch() < 0)
				damageStr = weapon->getDamageThrown();
		}
	}

	// TODO: Hit penalty for invisible target

	// TODO: [Bug] The player never has 'isRunning' flag
	if (!target.hasFlag(Actor::IsRunning)) // asleep or held
	{
		hitPlus += 4;
#ifdef _DEBUG
		std::wcout << L"![Run]";
#endif
	}

	std::istringstream iss(damageStr);
	std::string str;

	bool didHit = false;

	while (std::getline(iss, str, '/'))
	{
		std::istringstream iss2(str);

		int ndice;
		int nsides;
		char x;

		iss2 >> ndice >> x >> nsides;

#ifdef _DEBUG
		std::wcout << actor.getTheName();
#endif

		if (swing(actor.getLevel(), target.getArmor(), hitPlus + hitBonus[actor.getStr()]))
		{
			int proll = rollDice(ndice, nsides);
			int damage = proll + damagePlus + damageBonus[actor.getStr()];

			target.takeDamage(std::max(0, damage));
			
			didHit = true;
		}
	}

	target.startRun();

	int i = randomInt(4);

	if (actor.getType() != Actor::Hero)
		i += 4;

	if (didHit)
	{
		if (thrown)
		{
			// thunk: A missile hits a monster
			if (weapon->getType() == ItemType::Weapon)
			{
				Word theName(L"the " + weapon->getName(), weapon->getColor());
				message(L"{0} hits {1}.", { theName, target });
			}

			else
				message(L"{0} hit {1}.", { actor, target });

			weapon->reduceCount(1);

			if (actor.getType() == Actor::Hero)
				playSound(SoundID::RogueHit);
			else
				playSound(SoundID::MonsterHit);
		}

		else
		{
			// hit: Print a message to indicate a succesful hit
			if (actor.getChar() != L'I')
			{
				message(L"{0}{1}{2}.", { actor, hitStrings[i], target });

				if (actor.getType() == Actor::Hero)
					playSound(SoundID::RogueHit);
				else
					playSound(SoundID::MonsterHit);
			}
		}

		if (actor.hasFlag(Actor::CanConfuse))
		{
			actor.removeFlag(Actor::CanConfuse);
			target.addFlag(Actor::IsConfused);

			if (actor.getType() == Actor::Hero)
			{
				message(L"your hands stop glowing " + Potion::pickColor(actor, L"red") + L".");

				if (!target.isDead() && !actor.hasFlag(Actor::IsBlind))
					message(L"{0} appears confused.", { target });
			}
		}

		if (target.isDead())
			actor.killed(target);
		else
			actor.attack(target);
	}

	else
	{
		if (thrown)
		{
			// bounce: A missile misses a monster
			if (weapon->getType() == ItemType::Weapon)
			{
				Word theName(L"the " + weapon->getName(), weapon->getColor());
				message(L"{0} misses {1}.", { theName, target });
			}

			else
				message(L"{0} missed {1}.", { actor, target });

			if (actor.getType() == Actor::Hero)
				playSound(SoundID::RogueMiss);
			else
				playSound(SoundID::MonsterMiss);
		}

		else
		{
			// miss: Print a message to indicate a poor swing
			if (actor.getChar() != L'I')
			{
				message(L"{0}{1} {2}.", { actor, missStrings[i], target });

				if (actor.getType() == Actor::Hero)
					playSound(SoundID::RogueMiss);
				else
					playSound(SoundID::MonsterMiss);
			}
		}

		if (actor.getChar() == L'F')
			target.engulf();
	}

	return true;
}

bool AttackAction::swing(int level, int armor, int wplus)
{
	/* fight.c */
	// swing: Returns true if the swing hits

	int res = randomInt(20);
	int need = (20 - level) - armor;

#ifdef _DEBUG
	const int min = wplus - need;
	const int max = 19 + wplus - need;

	if (min >= 0)
		std::wcout << L" hit. (100%) ";
	else if (max < 0)
		std::wcout << L" missed. (0%)\n";
	else if (res + wplus >= need)
		std::wcout << L" hit. (" << (max + 1) * 100 / 20 << L"%) ";
	else
		// std::wcout << L" missed. (" << (-min) * 100 / 20 << L"%)\n";
		std::wcout << L" missed. (" << (max + 1) * 100 / 20 << L"%)\n";
#endif

	return res + wplus >= need;
}
