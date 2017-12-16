#include "Ring.hpp"
#include "../World.hpp"
// #include "../Actor.hpp"
#include "../../Utility/Rng.hpp"
#include "../../Utility/Utility.hpp"

#include <sstream>
#include <cassert>

namespace
{
	/* extern.c */
	std::vector<Item::Info> rings =
	{
		{ L"protection",         9, 400 },
		{ L"add strength",       9, 400 },
		{ L"sustain strength",   5, 280 },
		{ L"searching",         10, 420 },
		{ L"see invisible",     10, 310 },
		{ L"adornment",          1,  10 },
		{ L"aggravate monster", 10,  10 },
		{ L"dexterity",          8, 440 },
		{ L"increase damage",    8, 400 },
		{ L"regeneration",       4, 460 },
		{ L"slow digestion",     9, 240 },
		{ L"teleportation",      5,  30 },
		{ L"stealth",            7, 470 },
		{ L"maintain armor",     5, 380 },
	};

	/* init.c */
	struct Stone
	{
		std::wstring name;
		int value;
	};

	std::vector<Stone> stones =
	{
		{ L"agate",           25 },
		{ L"alexandrite",     40 },
		{ L"amethyst",        50 },
		{ L"carnelian",       40 },
		{ L"diamond",        300 },
		{ L"emerald",        300 },
		{ L"germanium",      225 },
		{ L"granite",          5 },
		{ L"garnet",          50 },
		{ L"jade",           150 },
		{ L"kryptonite",     300 },
		{ L"lapis lazuli",    50 },
		{ L"moonstone",       50 },
		{ L"obsidian",        15 },
		{ L"onyx",            60 },
		{ L"opal",           200 },
		{ L"pearl",          220 },
		{ L"peridot",         63 },
		{ L"ruby",           350 },
		{ L"sapphire",       285 },
		{ L"stibotantalite", 200 },
		{ L"tiger eye",       50 },
		{ L"topaz",           60 },
		{ L"turquoise",       70 },
		{ L"taaffeite",      300 },
		{ L"zircon",          80 },
	};
}

std::wstring Ring::getName() const
{
	std::wstring name;

	if (rings[which].know)
		name = L"ring of " + rings[which].name;
	else
		name = stones[which].name + L" ring";

	return name;
}

std::wstring Ring::getAName() const
{
	std::wostringstream oss;

	if (rings[which].know)
		oss << L"a ring of " << rings[which].name;
	else
		oss << (isVowel(stones[which].name) ? L"an " : L"a ") << stones[which].name << L" ring";

	/* rings.c */
	// ring_num: Print ring bonuses
	switch (which)
	{
	case Ring::Protect:
	case Ring::AddStrength:
	case Ring::AddHit:
	case Ring::AddDamage:
		if (hasFlag(Item::IsKnow))
			oss << L" [" << std::showpos << power << L"]";
		break;
	}

	return oss.str();
}

int Ring::getRingPower() const
{
	return power;
}

bool Ring::use(Actor& actor)
{
	/* rings.c */
	// ring_on: Put a ring on a hand

	Item* left = actor.getEquipment(Actor::LeftRing);
	Item* right = actor.getEquipment(Actor::RightRing);

	if (left == this)
	{
		if (dropCheck(actor))
			remove(actor, Actor::LeftRing);
	}

	else if (right == this)
	{
		if (dropCheck(actor))
			remove(actor, Actor::RightRing);
	}

	else if (!left)
	{
		// NOTE: Always remove more old ring first
		actor.setEquipment(Actor::LeftRing, right);
		actor.setEquipment(Actor::RightRing, nullptr);
		putOn(actor, Actor::RightRing);
	}

	else if (!right)
		putOn(actor, Actor::RightRing);

	else if (left->hasFlag(Item::IsCursed))
	{
		if (right->dropCheck(actor))
			putOn(actor, Actor::RightRing);
	}

	else if (right->hasFlag(Item::IsCursed))
	{
		if (left->dropCheck(actor))
			putOn(actor, Actor::LeftRing);
	}

	else
	{
		actor.setEquipment(Actor::LeftRing, nullptr);
		actor.setEquipment(Actor::LeftRing, right);
		actor.setEquipment(Actor::RightRing, nullptr);
		putOn(actor, Actor::RightRing);
	}

	return true;
}

void Ring::enchant()
{
	++power;
}

int Ring::ringEat() const
{
	/* rings.c */
	// ring_eat: How much food does this ring use up?
	
	const int uses[] =
	{
		 1, /* R_PROTECT  */	 1, /* R_ADDSTR   */
		 1, /* R_SUSTSTR  */	-3, /* R_SEARCH   */
		-5, /* R_SEEINVIS */	 0, /* R_NOP      */
		 0, /* R_AGGR     */	-3, /* R_ADDHIT   */
		-3, /* R_ADDDAM   */	 2, /* R_REGEN    */
		-2, /* R_DIGEST   */	 0, /* R_TELEPORT */
		 1, /* R_STEALTH  */	 1, /* R_SUSTARM  */
	};

	int eat = uses[which];

	if (eat < 0)
		eat = (randomInt(-eat) == 0);
	if (isSame(ItemType::Ring, Ring::Digest))
		eat = -eat;

	return eat;
}

void Ring::identify()
{
	rings[which].know = true;
	addFlag(Item::IsKnow);
}

void Ring::initStones(Rng& rng)
{
	/* init.c */
	// init_stones: Initialize the ring stone setting sceheme for this time

	rng.shuffle(stones);

	for (std::size_t i = 0; i < rings.size(); ++i)
		rings[i].worth += stones[i].value;
}

Item::Ptr Ring::createRing(int which, int power)
{
	auto ring = std::make_unique<Ring>(ItemType::Ring);
	ring->which = which;
	ring->power = power;

	if (power < 0)
		ring->addFlag(Item::IsCursed);

	return ring;
}

int Ring::pickOne(Rng& rng)
{
	return Item::pickOne(rng, rings);
}

void Ring::putOn(Actor& actor, int slot)
{
	assert(slot == Actor::LeftRing || slot == Actor::RightRing);
	
	actor.setEquipment(static_cast<Actor::Slot>(slot), this);
	message(L"you are now wearing {0}.", { *this });

	if (which == Ring::Aggravate)
		getWorld().aggravate();
}

void Ring::remove(Actor& actor, int slot)
{
	assert(slot == Actor::LeftRing || slot == Actor::RightRing);

	actor.setEquipment(static_cast<Actor::Slot>(slot), nullptr);
	message(L"you was wearing {0}.", { *this });
}
