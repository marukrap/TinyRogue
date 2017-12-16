#include "Armor.hpp"
#include "../Actor.hpp"
#include "../../Graphics/Color.hpp"

#include <sstream>

namespace
{
	/* extern.c */
	std::vector<Item::Info> armors =
	{
		{ L"leather armor",         20,  20 },
		{ L"ring mail",             15,  25 },
		{ L"studded leather armor", 15,  20 },
		{ L"scale mail",            13,  30 },
		{ L"chain mail",            12,  75 },
		{ L"splint mail",           10,  80 },
		{ L"banded mail",           10,  90 },
		{ L"plate mail",             5, 150 },
	};

	// Armor class for each armor type
	const std::vector<int> armorClass =
	{
		8, // LEATHER
		7, // RING_MAIL
		7, // STUDDED_LEATHER
		6, // SCALE_MAIL
		5, // CHAIN_MAIL
		4, // SPLINT_MAIL
		4, // BANDED_MAIL
		3, // PLATE_MAIL
	};
}

std::wstring Armor::getName() const
{
	return armors[which].name;
}

std::wstring Armor::getAName() const
{
	std::wostringstream oss;

	if (hasFlag(Item::IsKnow))
		oss << std::showpos << armor << L" " << armors[which].name << L" [" << std::noshowpos << 10 - getArmor() << L"]";
	else
		oss << armors[which].name;

	return oss.str();
}

int Armor::getArmor() const
{
	return armorClass[which] - armor;
}

bool Armor::use(Actor& actor)
{
	/* armor.c */
	// wear: The player wants to wear something, so let him/her put it on.

	Item* armor = actor.getEquipment(Actor::Armor);

	if (armor)
	{
		if (!armor->dropCheck(actor))
			return true; // false?

		else if (armor == this)
		{
			// Take off the current armor
			message(L"you used to be wearing {0}.", { *this });
			return true;
		}
	}

	actor.setEquipment(Actor::Armor, this);
	addFlag(Item::IsKnow);
	message(L"you are now wearing {0}.", { *this });

	return true;
}

void Armor::enchant()
{
	++armor;
}

void Armor::degrade()
{
	--armor;
}

Item::Ptr Armor::createArmor(int which, int ac)
{
	auto item = std::make_unique<Armor>(ItemType::Armor);
	item->which = which;
	item->armor = ac;

	if (ac < 0)
		item->addFlag(Item::IsCursed);

	return item;
}

int Armor::pickOne(Rng& rng)
{
	return Item::pickOne(rng, armors);
}
