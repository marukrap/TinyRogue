#include "Item.hpp"
#include "ActorPlayer.hpp" // REMOVE:
#include "../Graphics/Color.hpp"
#include "../Utility/Rng.hpp"
#include "../Utility/Utility.hpp"

// Items
#include "Items/Potion.hpp"
#include "Items/Scroll.hpp"
#include "Items/Weapon.hpp"
#include "Items/Armor.hpp"
#include "Items/Ring.hpp"
#include "Items/Stick.hpp"

#include <sstream>
#include <cassert>

namespace
{
	/* extern.c */
	std::vector<Item::Info> things =
	{
		{ L"potion", 26, L'!' },
		{ L"sroll",  36, L'?' },
		{ L"food",   16, L'%' },
		{ L"weapon",  7, L')' },
		{ L"armor",   7, L'[' },
		{ L"ring",    4, L'=' },
		{ L"stick",   4, L'/' },
		{ L"gold",    0, L'*' },
		{ L"amulet",  0, L'"' },
	};

	const std::wstring fruit = L"slime-mold";
}

int Item::Group = 2;

Item::Item(ItemType type)
	: Entity(things[static_cast<int>(type)].worth) // glyph
	, type(type)
{
	setColor(Color::Yellow);
}

std::wstring Item::getAName() const
{
	std::wostringstream oss;

	switch (type)
	{
	case ItemType::Food:
		if (which == 1)
		{
			if (count == 1)
				oss << (isVowel(fruit) ? L"an " : L"a ") << fruit;
			else
				oss << count << L" " << fruit << L"s";
		}

		else
		{
			if (count == 1)
				oss << L"some food";
			else
				oss << count << L" rations of food";
		}
		break;

	case ItemType::Gold:
		oss << count << L" gold pieces";
		break;

	case ItemType::Amulet:
		oss << L"the Amulet of Yendor";
		break;
	}

	return oss.str();
}

ItemType Item::getType() const
{
	return type;
}

bool Item::hasFlag(Flags flag) const
{
	return flags & flag;
}

void Item::addFlag(Flags flag)
{
	flags |= flag;
}

void Item::removeFlag(Flags flag)
{
	flags &= ~flag;
}

void Item::setCount(int count)
{
	this->count = count;
}

int Item::getCount() const
{
	return count;
}

void Item::addCount(int count)
{
	assert(count > 0);
	this->count += count;
}

void Item::reduceCount(int count)
{
	assert(count > 0);
	this->count -= count;
}

int Item::getWhich() const
{
	return which;
}

int Item::getGroup() const
{
	return group;
}

int Item::getLaunch() const
{
	return -1;
}

std::string Item::getDamage() const
{
	return "";
}

std::string Item::getDamageThrown() const
{
	return "";
}

int Item::getHitPlus() const
{
	return 0;
}

int Item::getDamagePlus() const
{
	return 0;
}

int Item::getArmor() const
{
	return 0;
}

int Item::getRingPower() const
{
	return 0;
}

bool Item::isSame(ItemType type, int which) const
{
	return this->type == type && this->which == which;
}

bool Item::isSame(Item& item) const
{
	return type == item.type && which == item.which;
}

bool Item::use(Actor& actor)
{
	if (type == ItemType::Food)
	{
		eat(actor);
		playSound(SoundID::Eat);
	}

	return true;
}

void Item::enchant()
{
	// Do nothing by default
}

void Item::degrade()
{
	// Do nothing by default
}

int Item::ringEat() const
{
	return 0;
}

void Item::eat(Actor& actor)
{
	// HACK: Avoid dynamic_cast
	dynamic_cast<ActorPlayer&>(actor).eatFood();

	if (which == 1)
		message(L"my, that was a yummy " + fruit + L".", Color::White);

	else
	{
		if (randomInt(100) > 70)
		{
			message(actor.chooseStr(L"bummer", L"yuk") + L", this food tastes awful.", Color::White);
			actor.gainExp(1);
		}

		else
			message(actor.chooseStr(L"oh, wow", L"yum") + L", that tasted good.", Color::White);
	}

	--count;
}

void Item::identify()
{
	addFlag(Item::IsKnow);
}

bool Item::isIdentified() const
{
	switch (type)
	{
	case ItemType::Weapon:
	case ItemType::Armor:
	case ItemType::Ring:
	case ItemType::Stick:
		return hasFlag(Item::IsKnow);
	}

	return true;
}

bool Item::isMagic() const
{
	/* potion.c */
	// is_magic: Returns true if an object radiates magic

	switch (type)
	{
	case ItemType::Potion:
	case ItemType::Scroll:
	case ItemType::Stick: // TODO: empty staff or wand?
	case ItemType::Ring:
	case ItemType::Amulet:
		return true;

	case ItemType::Weapon:
		return getHitPlus() > 0 || getDamagePlus() > 0;

	case ItemType::Armor:
		return hasFlag(Item::IsProtected) || getArmor() < 0;
	}

	return false;
}

bool Item::dropCheck(Actor& actor) const
{
	/* things.c */
	// dropcheck: Do special checks for dropping or unweilding|unwearing|unringing

	for (int i = 0; i < Actor::MaxSlot; ++i)
	{
		auto slot = static_cast<Actor::Slot>(i);

		if (actor.getEquipment(slot) == this)
		{
			if (hasFlag(Item::IsCursed))
			{
				Item* item = actor.getEquipment(slot);

				// message(L"you can't.  It appears to be cursed.", Color::White);
				message(L"you can't.  Your " + item->getName() + L" appears to be cursed.", Color::White);
				return false;
			}

			else
				actor.setEquipment(slot, nullptr); // unequip

			break;
		}
	}

	return true;
}

void Item::repaint()
{
	setChar(things[static_cast<int>(type)].worth);
	setColor(Color::Yellow);
}

void Item::initItems(Rng& rng)
{
	Potion::initColors(rng);
	Scroll::initNames(rng);
	Stick::initMaterials(rng);
	Ring::initStones(rng);
	// Group = 2;
}

Item::Ptr Item::detachOne()
{
	assert(count > 1);
	count -= 1;

	// Potion, scroll, food
	auto item = createItem(type, which);
	item->count = 1;
	item->flags = flags;

	return item;
}

Item::Ptr Item::createItem(ItemType type, int which)
{
	Item::Ptr item = nullptr;

	switch (type)
	{
	case ItemType::Potion:
		item = std::make_unique<Potion>(type);
		item->which = which;
		break;

	case ItemType::Scroll:
		item = std::make_unique<Scroll>(type);
		item->which = which;
		break;

	case ItemType::Food:
		item = std::make_unique<Item>(type);
		item->which = which;
		break;

	case ItemType::Weapon:
		item = Weapon::createWeapon(which);
		break;

	case ItemType::Armor:
		item = Armor::createArmor(which);
		break;

	case ItemType::Ring:
		item = Ring::createRing(which);
		break;

	case ItemType::Stick:
		item = Stick::createStick(which);
		break;
	}

	return item;
}

Item::Ptr Item::createItem(Rng& rng, int& noFood)
{
	/* things.c */
	// new_thing: Return a new thing

	auto type = noFood > 3 ? ItemType::Food : static_cast<ItemType>(pickOne(rng, things));
	Item::Ptr item = nullptr;

	switch (type)
	{
	case ItemType::Potion:
		item = std::make_unique<Potion>(type);
		item->which = Potion::pickOne(rng);
		break;

	case ItemType::Scroll:
		item = std::make_unique<Scroll>(type);
		item->which = Scroll::pickOne(rng);
		break;

	case ItemType::Food:
		noFood = 0;
		item = std::make_unique<Item>(type);
		item->which = rng.getInt(10) > 0 ? 0 : 1;
		break;

	case ItemType::Weapon:
	{
		int r = rng.getInt(100);
		int hitPlus = 0;

		if (r < 20) // 10 > 20 (10% > 20%)
			hitPlus -= rng.getInt(3) + 1;
		else if (r < 35) // 15 > 35 (5% > 15%)
			hitPlus += rng.getInt(3) + 1;

		item = Weapon::createWeapon(Weapon::pickOne(rng), hitPlus);
	}
		break;

	case ItemType::Armor:
	{
		int r = rng.getInt(100);
		int ac = 0;

		if (r < 25) // 20 > 25 (20% > 25%)
			ac += rng.getInt(3) + 1;
		else if (r < 43) // 28 > 43 (8% > 18%)
			ac -= rng.getInt(3) + 1;

		item = Armor::createArmor(Armor::pickOne(rng), ac);
	}
		break;

	case ItemType::Ring:
	{
		int power = 0;
		int which = Ring::pickOne(rng);

		switch (which)
		{
		case Ring::Protect:
		case Ring::AddStrength:
		case Ring::AddHit:
		case Ring::AddDamage:
			if ((power = rng.getInt(3)) == 0)
				power = -1;
			break;

		case Ring::Aggravate:
		case Ring::Teleport:
			power = -1;
			break;
		}

		item = Ring::createRing(which, power);
	}
		break;

	case ItemType::Stick:
		item = Stick::createStick(Stick::pickOne(rng));
		break;
	}

	return item;
}

Item::Ptr Item::createGold(int value)
{
	auto item = std::make_unique<Item>(ItemType::Gold);

	item->count = value;
	item->addFlag(Item::IsMany);
	item->group = 1; // GOLDGRP 1

	return item;
}

std::wstring Item::getFruit()
{
	return fruit;
}

wchar_t Item::getRandomChar()
{
	// TODO: Add stairs
	return things[randomInt(things.size())].worth;
}

int Item::pickOne(Rng& rng, std::vector<Info>& items)
{
	int totalProb = 0;

	for (const auto& item : items)
		totalProb += item.prob;

	int i = rng.getInt(totalProb); // 100

	for (std::size_t j = 0; j < items.size(); ++j)
	{
		i -= items[j].prob;

		if (i < 0)
			return j;
	}

	return -1; // error
}
