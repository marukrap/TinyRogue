#include "Weapon.hpp"
#include "../Actor.hpp"
#include "../../Graphics/Color.hpp"
#include "../../Utility/Utility.hpp"

#include <sstream>
#include <cassert>

namespace
{
	/* extern.c */
	std::vector<Item::Info> weapons =
	{
		{ L"mace",             11,  8 },
		{ L"long sword",       11, 15 },
		{ L"short bow",        12, 15 },
		{ L"arrow",            12,  1 },
		{ L"dagger",            8,  3 },
		{ L"two handed sword", 10, 75 },
		{ L"dart",             12,  2 },
		{ L"shuriken",         12,  5 },
		{ L"spear",            12,  5 },
		{ L"",                  0,  0 }, // DO NOT REMOVE: fake entry for dragon's breath
	};

	/* weapons.c */
	struct WeaponData
	{
		std::string damage;			// Damage when wielded
		std::string damageThrown;	// Damage when thrown
		int launch;					// Launching weapon
		int flags;					// Miscellaneous flags
	};
	
	const int NoWeapon = -1;
	const int Bow = 2;

	std::vector<WeaponData> weaponInfo =
	{
		{ "2x4", "1x3", NoWeapon, 0,                               }, // Mace
		{ "3x4", "1x2", NoWeapon, 0,                               }, // Long sword
		{ "1x1", "1x1", NoWeapon, 0,                               }, // Bow
		{ "1x1", "2x3", Bow,      Item::IsMany|Item::IsMissile,    }, // Arrow
		{ "1x6", "1x4", NoWeapon, Item::IsMany|Item::IsMissile,    }, // Dagger
		{ "4x4", "1x2", NoWeapon, 0,                               }, // 2h sword
		{ "1x1", "1x3", NoWeapon, Item::IsMany|Item::IsMissile,    }, // Dart
		{ "1x2", "2x4", NoWeapon, Item::IsMany|Item::IsMissile,    }, // Shuriken
		{ "2x3", "1x6", NoWeapon, Item::IsMissile,                 }, // Spear
		{    "",    "", NoWeapon, 0,                               },
	};
}

std::wstring Weapon::getName() const
{
	return weapons[which].name;
}

std::wstring Weapon::getAName() const
{
	std::wostringstream oss;

	// HACK: The flame, the bolt, ...
	if (which == Weapon::Flame)
		return L"the " + weapons[which].name;

	if (count > 1)
		oss << count << L" ";
	else
		oss << (isVowel(weapons[which].name) ? L"an " : L"a ");

	if (hasFlag(Item::IsKnow))
		oss << std::showpos << hitPlus << L"," << damagePlus << L" ";

	oss << weapons[which].name;

	if (count > 1)
		oss << L"s";

	return oss.str();
}

int Weapon::getLaunch() const
{
	return weaponInfo[which].launch;
}

std::string Weapon::getDamage() const
{
	return weaponInfo[which].damage;
}

std::string Weapon::getDamageThrown() const
{
	return weaponInfo[which].damageThrown;
}

int Weapon::getHitPlus() const
{
	return hitPlus;
}

int Weapon::getDamagePlus() const
{
	return damagePlus;
}

#include "../World.hpp"

bool Weapon::use(Actor& actor)
{
	/* weapons.c */
	// wield: Pull out a certain weapon

	Item* weapon = actor.getEquipment(Actor::Weapon);

	if (weapon)
	{
		if (!weapon->dropCheck(actor))
			return true; // false?

		else if (weapon == this)
		{
			// Unwield the current weapon
			message(L"you used to be wieling {0}.", { *this });
			return true;
		}
	}
	
	actor.setEquipment(Actor::Weapon, this);
	message(L"you are now wielding {0}.", { *this });

	return true;
}

void Weapon::enchant()
{
	if (randomInt(2) == 0)
		++hitPlus;
	else
		++damagePlus;
}

Item::Ptr Weapon::detachOne()
{
	assert(count > 1);
	count -= 1;

	auto weapon = std::make_unique<Weapon>(type);
	weapon->which = which;
	weapon->flags = flags;
	weapon->hitPlus = hitPlus;
	weapon->damagePlus = damagePlus;
	weapon->count = 1;
	weapon->group = group;

	return weapon;
}

Item::Ptr Weapon::createWeapon(int which, int hitPlus, int damagePlus)
{
	/* weapons.c */
	// init_weapon: Set up the initial goodies for a weapon

	auto weapon = std::make_unique<Weapon>(ItemType::Weapon);
	weapon->which = which;
	weapon->flags = weaponInfo[which].flags;
	weapon->hitPlus = hitPlus;
	weapon->damagePlus = damagePlus;

	if (hitPlus < 0 || damagePlus < 0)
		weapon->addFlag(Item::IsCursed);

	if (which == Weapon::Dagger)
	{
		weapon->count = randomInt(4) + 2;
		weapon->group = Group++;
	}

	else if (weapon->hasFlag(Item::IsMany))
	{
		weapon->count = randomInt(8) + 8;
		weapon->group = Group++;
	}

	else
	{
		weapon->count = 1;
		weapon->group = 0;
	}

	return weapon;
}

Item::Ptr Weapon::createBolt(wchar_t ch, const sf::Color& color, const std::wstring& name, const std::string& damage, int hitPlus, int damagePlus, bool isMissile)
{
	auto bolt = std::make_unique<Weapon>(ItemType::Weapon);
	bolt->setChar(ch);
	bolt->setColor(color);
	bolt->which = Weapon::Flame;
	bolt->hitPlus = hitPlus;
	bolt->damagePlus = damagePlus;

	if (isMissile)
		bolt->addFlag(Item::IsMissile);

	weapons[Weapon::Flame].name = name;
	weaponInfo[Weapon::Flame].damageThrown = damage;

	return bolt;
}

int Weapon::pickOne(Rng& rng)
{
	return Item::pickOne(rng, weapons);
}
