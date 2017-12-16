#include "Potion.hpp"
#include "Ring.hpp"
#include "../../World/World.hpp"
#include "../../Graphics/Color.hpp"
#include "../../Utility/Utility.hpp"

#include <sstream>

namespace
{
	/* extern.c */
	std::vector<Item::Info> potions =
	{
		{ L"confusion",          7,   5 },
		{ L"hallucination",      8,   5 },
		{ L"poison",             8,   5 },
		{ L"gain strength",     13, 150 },
		{ L"see invisible",      3, 100 },
		{ L"healing",           13, 130 },
		{ L"monster detection",  6, 130 },
		{ L"magic detection",    6, 105 },
		{ L"raise level",        2, 250 },
		{ L"extra healing",      5, 200 },
		{ L"haste self",         5, 190 },
		{ L"restore strength",  13, 130 },
		{ L"blindness",          5,   5 },
		{ L"levitation",         6,  75 },
	};

	/* init.c */
	std::vector<std::wstring> rainbow =
	{
		L"amber",
		L"aquamarine",
		L"black",
		L"blue",
		L"brown",
		L"clear",
		L"crimson",
		L"cyan",
		L"ecru",
		L"gold",
		L"green",
		L"grey",
		L"magenta",
		L"orange",
		L"pink",
		L"plaid",
		L"purple",
		L"red",
		L"silver",
		L"tan",
		L"tangerine",
		L"topaz",
		L"turquoise",
		L"vermilion",
		L"violet",
		L"white",
		L"yellow",
	};
	
	const int HuhDuration = 20;
	const int SeeDuration = 850;
	const int HealTime = 30;
}

std::wstring Potion::getAName() const
{
	std::wostringstream oss;

	if (potions[which].know)
	{
		if (count == 1)
			oss << L"a potion";
		else
			oss << count << L" potions";

		oss << L" of " << potions[which].name;
	}

	else
	{
		if (count == 1)
			oss << (isVowel(rainbow[which]) ? L"an " : L"a ");
		else
			oss << count << L" ";

		oss << rainbow[which] << L" potion";

		if (count > 1)
			oss << L"s";
	}

	return oss.str();
}

bool Potion::use(Actor& actor)
{
	/* potions.c */
	// quaff: Quaff a potion from the pack

	playSound(SoundID::Eat);

	bool trip = actor.hasFlag(Actor::IsHallucinating);

	switch (which)
	{
	case Potion::Confuse:
		quaff(actor, which, !trip, Actor::IsConfused, HuhDuration);
		message(actor.chooseStr(L"what a tripy feeling!",
								L"wait, what's going on here. Huh? What? Who?"), Color::White);
		break;

	case Potion::Hallucinate:
		quaff(actor, which, true, Actor::IsHallucinating, SeeDuration);
		break;

	case Potion::Poison:
		potions[which].know = true;

		if (actor.hasRing(Ring::SustainStrength))
		{
			message(L"you feel momentarily sick.", Color::White);
			// TODO: Identify the ring if it is not identified
		}

		else
		{
			actor.changeStr(-(randomInt(3) + 1));
			actor.removeEffect(Actor::IsHallucinating); // come_down:
			message(L"you feel very sick now.", Color::White);
		}
		break;

	case Potion::Strength:
		potions[which].know = true;
		actor.changeStr(1);
		message(L"you feel stronger, now.  What bulging muscles!", Color::White);
		break;

	case Potion::SeeInvisible:
		quaff(actor, which, false, Actor::CanSee, SeeDuration);

		if (actor.hasFlag(Actor::IsBlind))
		{
			potions[which].know = true;
			actor.removeEffect(Actor::IsBlind); // sight:
		}

		message(L"this potion tastes like " + Item::getFruit() + L" juice.", Color::White);
		break;

	case Potion::Healing:
		potions[which].know = true;
		actor.potionHeal();
		actor.removeEffect(Actor::IsBlind); // sight:
		message(L"you begin to feel better.", Color::White);
		break;

	case Potion::MonsterFind:
	{
		actor.addEffect(Actor::SeeMonster, HuhDuration);

		// turn_see: Put on or off seeing monsters on this level
		bool found = false;

		getLevel().foreachActors([&] (Actor& monster)
		{
			if (monster.getPosition() != actor.getPosition())
				found = true;
		});

		if (found)
			potions[which].know = true;
		else
			message(L"you have a " + actor.chooseStr(L"normal", L"strange") + L" feeling for a moment, then it passes.", Color::White);
	}
		break;

	case Potion::MagicFind:
	{
		// TODO: Soft-id the items in the pack

		bool found = false;

		getLevel().foreachItems([&found] (Item& item)
		{
			if (item.isMagic())
			{
				item.addFlag(Item::IsDetected);
				found = true;
			}
		});

		if (found)
		{
			potions[which].know = true;
			message(L"you sense the presence of magic on this level.", Color::White);
		}

		else
			message(L"you have a " + actor.chooseStr(L"normal", L"strange") + L" feeling for a moment, then it passes.", Color::White);
	}
		break;

	case Potion::RaiseLevel:
		potions[which].know = true;
		message(L"you suddenly feel much more skillful.", Color::White);
		actor.raiseLevel();
		break;

	case Potion::ExtraHeal:
		potions[which].know = true;
		actor.potionHeal(true);
		actor.removeEffect(Actor::IsBlind); // sight:
		actor.removeEffect(Actor::IsHallucinating); // come_down:
		message(L"you begin to feel much better.", Color::White);
		break;

	case Potion::Haste:
		potions[which].know = true;

		// add_haste: Add a haste to the player
		if (actor.hasFlag(Actor::IsHasted))
		{
			actor.removeEffect(Actor::IsHasted);
			actor.addEffect(Actor::NoCommand, randomInt(8));
			message(L"you faint from exhaustion.", Color::White);
		}

		else
		{
			actor.addEffect(Actor::IsHasted, randomInt(4) + 4);
			message(L"you feel yourself moving much faster.", Color::White);
		}
		break;

	case Potion::Restore:
		if (actor.restoreStr())
			potions[which].know = true;

		message(L"hey, this tastes great.  It make you feel warm all over.", Color::White);
		break;
		
	case Potion::Blind:
		quaff(actor, which, true, Actor::IsBlind, SeeDuration);
		break;

	case Potion::Levitate:
		quaff(actor, which, true, Actor::IsLevitating, HealTime);
		break;

	default:
		message(L"what an odd tasting potion!", Color::White);
		break;
	}

	--count;

	return true;
}

void Potion::identify()
{
	potions[which].know = true;
}

bool Potion::isIdentified() const
{
	return potions[which].know;
}

void Potion::initColors(Rng& rng)
{
	/* init.c */
	// init_colors: Initialize the potion color scheme for this time

	rng.shuffle(rainbow);
}

std::wstring Potion::randomColor()
{
	return rainbow[randomInt(rainbow.size())];
}

std::wstring Potion::pickColor(Actor& actor, const std::wstring& color)
{
	if (actor.hasFlag(Actor::IsHallucinating))
		return randomColor();

	return color;
}

int Potion::pickOne(Rng& rng)
{
	return Item::pickOne(rng, potions);
}

void Potion::quaff(Actor& actor, int type, bool knowit, int flag, int time)
{
	// do_pot: Do a potion with standard setup.
	//         This means it uses a fuse and turns on a flag

	if (!potions[type].know)
		potions[type].know = knowit;

	actor.addEffect(static_cast<Actor::Flags>(flag), spread(time));
}
