#include "Trap.hpp"
#include "Actor.hpp"
#include "../Graphics/Color.hpp"
#include "../Utility/Utility.hpp" // randomInt
#include "../Action/AttackAction.hpp" // swing

#include "Items/Potion.hpp" // randomColor
#include "Items/Ring.hpp"

#include <array>

namespace
{
	std::array<std::wstring, Trap::TrapCount> trapNames =
	{
		L"a trapdoor",
		L"an arrow trap",
		L"a sleeping gas trap",
		L"a beartrap",
		L"a teleport trap",
		L"a poison dart trap",
		L"a rust trap",
		L"a mysterious trap",
	};
}

Trap::Trap(Type type)
	: Entity(L'^')
	, type(type)
{
	// TODO: Trap color?
}

bool Trap::isActive() const
{
	return active;
}

void Trap::activate()
{
	active = true;
}

void Trap::deactivate()
{
	active = false;
}

/* move.c */
// be_trapped: The guy stepped on a trap.... Make him pay.
void Trap::beTrapped(Actor& actor)
{
	playSound(SoundID::Trap);

	activate();

	switch (type)
	{
	case Trap::Door:
		message(L"you fell into a trap!", Color::White);
		descend();
		teleport(actor);
		break;

	case Trap::Arrow:
		if (AttackAction::swing(actor.getLevel() - 1, actor.getArmor(), 1))
		{
			actor.takeDamage(rollDice(1, 6));

			if (actor.isDead())
				message(L"an arrow killed you.", Color::Red); // death('a');
			else
				message(L"oh no! An arrow shot you.", Color::Red);
			
			setColor(Color::Blood);
		}

		else
		{
			// TODO: Drop an arrow
			// arrow = new_item();
			// init_weapon(arrow, ARROW);
			// arrow->o_count = 1;
			// arrow->o_pos = hero;
			// fall(arrow, FALSE);

			message(L"an arrow shoots past you.", Color::White);
		}
		break;

	case Trap::Sleep:
		actor.addEffect(Actor::NoCommand, spread(5)); // SLEEPTIME spread(5)
		message(L"a strange white mist envelops you and you fall asleep.", Color::White);
		break;

	case Trap::Bear:
		actor.addEffect(Actor::NoMove, spread(3)); // BEARTIME spread(3)
		message(L"you are caught in a bear trap.", Color::White);
		break;

	case Trap::Telep:
		teleport(actor);
		break;

	case Trap::Dart:
		if (AttackAction::swing(actor.getLevel() + 1, actor.getArmor(), 1))
		{
			actor.takeDamage(rollDice(1, 4));

			if (actor.isDead())
				message(L"a poisoned dart killed you.", Color::Red); // death('d');
			else if (!actor.hasRing(Ring::SustainStrength) && !actor.resistCheck(Actor::Poison))
				actor.changeStr(-1);
			else
				message(L"a small dart just hit you in the shoulder.", Color::Red);

			setColor(Color::Blood);
		}

		else
			message(L"a small dart whizzes by your ear and vanishes.", Color::White);
		break;

	case Trap::Rust:
		message(L"a gush of water hits you on the head.", Color::White);
		actor.rustArmor();
		break;

	case Trap::Myst:
		switch (randomInt(11))
		{
		case  0: message(L"you are suddenly in a parallel dimension.", Color::White); break;
		case  1: message(L"the light in here suddenly seems " + Potion::randomColor() + L".", Color::White); break;
		case  2: message(L"you feel a sting in the side of your neck.", Color::White); break;
		case  3: message(L"multi-colored lines swirl around you, then fade.", Color::White); break;
		case  4: message(L"a " + Potion::randomColor() + L" light flashes in your eyes.", Color::White); break;
		case  5: message(L"a spike shoots past your ear!", Color::White); break;
		case  6: message(Potion::randomColor() + L" sparks dance across your armor.", Color::White); break;
		case  7: message(L"you suddenly feel very thirsty.", Color::White); break;
		case  8: message(L"you feel time speed up suddenly.", Color::White); break;
		case  9: message(L"time now seems to be going slower.", Color::White); break;
		case 10: message(L"you pack turns " + Potion::randomColor() + L"!", Color::White); break;
		}
		break;
	}
}
