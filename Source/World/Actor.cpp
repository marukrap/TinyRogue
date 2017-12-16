#include "Actor.hpp"
#include "World.hpp"
#include "../Game/Console.hpp"
#include "../Graphics/Color.hpp"
#include "../Utility/Rng.hpp"
#include "../Utility/Utility.hpp"

#include "Items/Armor.hpp"
#include "Items/Ring.hpp"

#include <algorithm> // find_if
#include <iostream>
#include <cassert>

namespace
{
	/* extern.c */
	struct Monster
	{
		std::wstring name;	// What to call the monster
		int carry;			// Probability of carrying something
		int flags;			// things about the monster
		Actor::Stats stats;	// Initial stats
		sf::Color color;
	};

	std::vector<Monster> monsters =
    {
		// Name          CARRY    FLAG                             exp, lvl, amr, dmg               Color
		{ L"aquator",        0,   Actor::IsMean,                {   20,   5,   2, "0x0/0x0"      }, Color::Green }, // Blue?
		{ L"bat",            0,   Actor::IsFly,                 {    1,   1,   3, "1x2"          }, Color::DarkGray },
		{ L"centaur",       15,   0,                            {   17,   4,   4, "1x2/1x5/1x5"  }, Color::Brown },
		{ L"dragon",       100,   Actor::IsMean,                { 5000,  10,  -1, "1x8/1x8/3x10" }, Color::DarkGreen },
		{ L"emu",            0,   Actor::IsMean,                {    2,   1,   7, "1x2"          }, Color::LightGray }, // TODO: change color
		{ L"venus flytrap",  0,   Actor::IsMean,                {   80,   8,   3, "%%%x0"        }, Color::Red },
		{ L"griffin",       20,   Actor::IsMean|Actor::IsFly|Actor::IsRegen, { 2000,  13,   2, "4x3/3x5"      }, Color::DarkBrown },
		{ L"hobgoblin",      0,   Actor::IsMean,                {    3,   1,   5, "1x8"          }, Color::DarkBrown },
		{ L"ice monster",    0,   0,                            {    5,   1,   9, "0x0"          }, Color::Cyan },
		{ L"jabberwock",    70,   0,                            { 3000,  15,   6, "2x12/2x4"     }, Color::Gray }, // TODO: change color
		{ L"kestrel",        0,   Actor::IsMean| Actor::IsFly,  {    1,   1,   7, "1x4"          }, Color::Brown },
		{ L"leprechaun",     0,   0,                            {   10,   3,   8, "1x1"          }, Color::Pink },
		{ L"medusa",        40,   Actor::IsMean,                {  200,   8,   2, "3x4/3x4/2x5"  }, Color::DarkGreen },
		{ L"nymph",        100,   0,                            {   37,   3,   9, "0x0"          }, Color::Blue },
		{ L"orc",           15,   Actor::IsGreed,               {    5,   1,   6, "1x8"          }, Color::DarkGreen },
		{ L"phantom",        0,   Actor::IsInvisible,           {  120,   8,   3, "4x4"          }, sf::Color(255, 255, 255) }, // NOTE: sf::Color::White is not initialized at this time
		{ L"quagga",         0,   Actor::IsMean,                {   15,   3,   3, "1x5/1x5"      }, Color::Brown },
		{ L"rattlesnake",    0,   Actor::IsMean,                {    9,   2,   3, "1x6"          }, Color::DarkBrown },
		{ L"snake",          0,   Actor::IsMean,                {    2,   1,   5, "1x3"          }, Color::Green },
		{ L"troll",         50,   Actor::IsRegen|Actor::IsMean, {  120,   6,   4, "1x8/1x8/2x6"  }, Color::Green },
		{ L"black unicorn",  0,   Actor::IsMean,                {  190,   7,  -2, "1x9/1x9/2x9"  }, Color::DarkGray },
		{ L"vampire",       20,   Actor::IsRegen|Actor::IsMean, {  350,   8,   1, "1x10"         }, Color::Red },
		{ L"wraith",         0,   0,                            {   55,   5,   4, "1x6"          }, sf::Color(255, 255, 255) },
		{ L"xeroc",         30,   0,                            {  100,   7,   7, "4x4"          }, Color::Yellow },
		{ L"yeti",          30,   0,                            {   50,   4,   6, "1x6/1x6"      }, Color::Cyan },
		{ L"zombie",         0,   Actor::IsMean,                {    6,   2,   8, "1x8"          }, Color::Gray }
	};

	/* monsters.c */
	// List of monsters in rough order of vorpalness
	const std::vector<int> dwellers =
	{
		'K', 'E', 'B', 'S', 'H', 'I', 'R', 'O', 'Z', 'L', 'C', 'Q', 'A',
		'N', 'Y', 'F', 'T', 'W', 'P', 'X', 'U', 'M', 'V', 'G', 'J', 'D'
	};

	const std::vector<int> wanderers =
	{
		'K', 'E', 'B', 'S', 'H',   0, 'R', 'O', 'Z',   0, 'C', 'Q', 'A',
		  0, 'Y',   0, 'T', 'W', 'P',   0, 'U', 'M', 'V', 'G', 'J',   0
	};

	/* extern.c */
	const std::vector<int> expTable =
	{
		10L,
		20L,
		40L,
		80L,
		160L,
		320L,
		640L,
		1300L,
		2600L,
		5200L,
		13000L,
		26000L,
		50000L,
		100000L,
		200000L,
		400000L,
		800000L,
		2000000L,
		4000000L,
		8000000L,
		0L
	};

	const int AmuletLevel = 26;
	const int BoreLevel = 50;

	int VfHit = 0;
}

Actor::Actor(wchar_t ch)
	: Entity(ch)
	, disguise(ch)
{
	equipment.fill(nullptr);
}

std::wstring Actor::getAName() const
{
	std::wstring result;

	if (type == Actor::Hero)
		result = L"you";
	else if (!getWorld().isVisible(*this))
		result = L"something";
	else
	{
		// HACK: Xeroc
		if (disguise < L'A' || disguise > L'Z')
			result = L"a " + monsters['X'].name;
		else
			result = L"a " + monsters[disguise - L'A'].name;
	}

	return result;
}

std::wstring Actor::getTheName() const
{
	std::wstring result;

	if (type == Actor::Hero)
		result = L"you";
	else if (!getWorld().isVisible(*this))
		result = L"something";
	else
	{
		// HACK: Xeroc
		if (disguise < L'A' || disguise > L'Z')
			result = L"the " + monsters['X'].name;
		else
			result = L"the " + monsters[disguise - L'A'].name;
	}

	return result;
}

const sf::Color& Actor::getColor() const
{
	if (type != Actor::Hero && !getWorld().isVisible(*this))
		return sf::Color::White;
	
	return Entity::getColor();
}

Actor::Type Actor::getType() const
{
	return type;
}

void Actor::setDisguise(wchar_t ch)
{
	disguise = ch;
}

wchar_t Actor::getDisguise() const
{
	return disguise;
}

bool Actor::hasFlag(Flags flag) const
{
	return flags & flag;
}

void Actor::addFlag(Flags flag)
{
	flags |= flag;
}

void Actor::removeFlag(Flags flag)
{
	flags &= ~flag;
}

int Actor::getExp() const
{
	return stats.exp;
}

int Actor::getLevel() const
{
	return stats.level;
}

int Actor::getHp() const
{
	return stats.hp;
}

int Actor::getMaxHp() const
{
	return stats.maxhp;
}

int Actor::getStr() const
{
	int str = stats.str;

	if (isEquipment(LeftRing, Ring::AddStrength))
		str += equipment[LeftRing]->getRingPower();
	if (isEquipment(RightRing, Ring::AddStrength))
		str += equipment[RightRing]->getRingPower();

	return str;
}

int Actor::getMaxStr() const
{
	return stats.maxstr;
}

int Actor::getArmor() const
{
	int armor = 0;

	if (equipment[Armor])
		armor = equipment[Armor]->getArmor();
	else
		armor = stats.armor;

	if (isEquipment(Actor::LeftRing, Ring::Protect))
		armor -= getEquipment(Actor::LeftRing)->getRingPower();
	if (isEquipment(Actor::RightRing, Ring::Protect))
		armor -= getEquipment(Actor::RightRing)->getRingPower();

	return armor;
}

std::string Actor::getDamage() const
{
	// if (equipment[Weapon])
		// return equipment[Weapon]->getDamage();

	return stats.damage;
}

void Actor::gainExp(int exp)
{
	stats.exp += exp;

	if (stats.exp > 9999999L)
		stats.exp = 9999999L;

	/* misc.c */
	// check_level: Check to see if the guy has gone up a level
	int i;

	for (i = 0; i < expTable[i] != 0; ++i)
		if (expTable[i] > stats.exp)
			break;

	if (++i > stats.level)
	{
		int add = rollDice(i - stats.level, 10);

		stats.level = i;
		stats.maxhp += add;
		stats.hp += add;

		message(L"welcome to level " + std::to_wstring(i) + L".", Color::Yellow);
		playSound(SoundID::Experience);
	}
}

void Actor::raiseLevel()
{
	/* potions.c */
	// raise_level: The guy just magically went up a level

	if (stats.level < static_cast<int>(expTable.size()))
		stats.exp = expTable[stats.level - 1];

	gainExp(1);
}

void Actor::lowerLevel()
{
	if (stats.level == 1)
		stats.exp = 0;

	else
	{
		stats.level -= 1;
		stats.exp = expTable[stats.level - 1] + 1;
	}
}

void Actor::takeDamage(int points)
{
	assert(points >= 0);

	stats.hp -= points;

	if (stats.hp < 0)
		stats.hp = 0;

#ifdef _DEBUG
	std::wcout << getTheName() << " took " << points << " damage. (HP:" << getHp() << "/" << getMaxHp() << ")\n";
#endif
}

void Actor::restoreHp(int points)
{
	assert(points >= 0);

	stats.hp = points;

	if (stats.hp > stats.maxhp)
		stats.hp = stats.maxhp;
}

void Actor::changeMaxHp(int points)
{
	stats.maxhp += points;
}

bool Actor::isDead() const
{
	return stats.hp <= 0;
}

bool Actor::isMarkedForRemoval() const
{
	return type != Actor::Hero && isDead();
}

void Actor::changeStr(int amount)
{
	/* misc.c */
	// chg_str: Used to modify the playes strength.  It keeps track of the
	//          highest it has been, just in case

	// add_str: Perform the actual add, checking upper and lower bound limits

	stats.str += amount;

	if (stats.str < 3)
		stats.str = 3;
	else if (stats.str > 31)
		stats.str = 31;

	if (stats.str > stats.maxstr)
		stats.maxstr = stats.str;
}

bool Actor::restoreStr()
{
	if (stats.str < stats.maxstr)
	{
		stats.str = stats.maxstr;
		return true;
	}

	return false;
}

void Actor::kill()
{
	stats.hp = 0;
}

void Actor::killed(Actor& target)
{
	/* fight.c */
	// killed: Call to put a monster to death

	if (target.getType() == Actor::Hero)
	{
		message(L"you died...", Color::Blood);
		playSound(SoundID::RogueDie);

		// Identify all items in the pack
		for (auto& item : target.pack)
			item->identify();
	}
	
	else if (type == Actor::Hero)
	{
		message(L"{0} have defeated {1}.", { *this, target });
		playSound(SoundID::MonsterDie);
		gainExp(target.getExp());

		if (target.getChar() == L'F')
			unhold();

		else if (target.getChar() == L'L')
		{
			sf::Vector2i pos = target.getPosition();
			std::vector<sf::Vector2i> fallPos;

			// fallpos: Pick a random position around the give (y, x) coordinates

			for (int dy = -1; dy <= 1; ++dy)
				for (int dx = -1; dx <= 1; ++dx)
				{
					sf::Vector2i dir(dx, dy);

					if (getPosition() != (pos + dir))
						continue;

					Tile::Type tile = getWorld().getLevel().at(pos + dir).type;

					if (tile == Tile::Floor && !getWorld().getLevel().itemAt(pos + dir))
						fallPos.emplace_back(pos + dir);
				}

			if (!fallPos.empty()) // && depth >= max_depth
			{
				pos = fallPos[randomInt(fallPos.size())];

				int depth = getWorld().getDepth();
				auto gold = Item::createGold(randomInt(50 + 10 * depth) + 2);

				if (resistCheck(Resist::Magic))
				{
					for (int i = 0; i < 4; ++i)
						gold->addCount(randomInt(50 + 10 * depth) + 2);
				}

				gold->setPosition(pos);
				getWorld().getLevel().attach(std::move(gold));
			}
		}
	}

	else
		message(L"{0} has defeated {1}.", { *this, target });
}

void Actor::death(const std::wstring& cause)
{
	// NOTE: Player only
	stats.hp = 0;
	message(L"you died by " + cause + L".", Color::Blood);
	playSound(SoundID::RogueDie);
}

void Actor::attack(Actor& target)
{
	/* fight.c */
	// attack: The monster attacks the player

	if (hasFlag(Actor::IsCancelled))
		return;

	// TODO: Use ability flags instead character
	switch (getChar())
	{
	case L'A':
		// If an aquator hits, you can lose armor class.
		target.rustArmor();
		break;

	case L'F':
		// Venus Flytrap stops the poor guy from moving
		target.addFlag(Actor::IsHeld);
		monsters[L'F' - L'A'].stats.damage = std::to_string(++VfHit) + "x1";

		target.takeDamage(1);

		if (target.isDead())
			target.death(getAName());
		break;

	case L'I':
		// The ice monster freezes you
		if (!target.hasFlag(Actor::NoCommand))
			message(L"you are frozen by {0}.", { *this });

		target.addEffect(Actor::NoCommand, randomInt(2) + 2);
		playSound(SoundID::Ice);

		if (target.getEffectDuration(Actor::NoCommand) > BoreLevel)
			target.death(L"hypothermia");
		break;

	case L'L':
		// Leperachaun steals some gold
	{
		int lastPurse = target.purse;
		target.purse -= randomInt(50 + 10 * getWorld().getDepth()) + 2; // GOLDCALC

		if (!target.resistCheck(Actor::Magic))
		{
			for (int i = 0; i < 4; ++i)
				target.purse -= randomInt(50 + 10 * getWorld().getDepth()) + 2; // GOLDCALC
		}

		if (target.purse < 0)
			target.purse = 0;

		// TODO: 몬스터를 지우지 않고 텔레포트 후 도망(flee) > 처치할 경우 골드 회수
		kill(); // remove_mon:

		if (target.purse != lastPurse)
		{
			message(L"your purse feels lighter.", Color::White);
			playSound(SoundID::Gold);
		}
	}
	break;

	case L'N':
		// Nymph's steal a magic item, look through the pack
		// and pick out one we like.
	{
		std::vector<Item*> items;

		for (const auto& item : target.pack)
		{
			bool equipped = false;

			for (int i = 0; i < Actor::MaxSlot; ++i)
			{
				if (target.getEquipment(static_cast<Actor::Slot>(i)) == item.get())
				{
					equipped = true;
					break;
				}
			}

			if (!equipped)
				items.emplace_back(item.get());
		}

		if (!items.empty())
		{
			Item* item = items[randomInt(items.size())];
			auto steal = target.unpack(*item);
			message(L"she stole {0}!", { *steal });

			// TODO: 몬스터를 지우지 않고 텔레포트 후 도망(flee) > 처치할 경우 아이템 회수
			kill(); // remove_mon:
		}
	}
	break;

	case L'R':
		// Rattlesnakes have poisonous bites
		if (!target.hasRing(Ring::SustainStrength) && !target.resistCheck(Actor::Poison))
		{
			target.changeStr(-1);
			message(L"you feel a bite in your leg and now feel weaker.", Color::Red);
		}

		else
			message(L"a bite momentarily weakens you.", Color::White);
		break;

	case L'V':
	case L'W':
		// Wraiths might drain energy levels, and Vampires can steal max_hp
		if (randomInt(100) < (getChar() == L'W' ? 15 : 30))
		{
			int fewer;

			if (getChar() == L'W')
			{
				if (target.getExp() == 0)
				{
					// TODO: Change the message
					target.death(getAName()); // All levels gone
					return;
				}

				else
					target.lowerLevel();

				fewer = rollDice(1, 10);
			}

			else
				fewer = rollDice(1, 3);

			target.takeDamage(fewer);
			target.changeMaxHp(-fewer);

			if (target.isDead())
				target.restoreHp(1);

			if (target.getMaxHp() <= 0)
			{
				target.kill();
				target.death(getAName());
			}

			else
				message(L"you suddenly feel weaker.", Color::White);
		}
		break;
	}
}

// TODO: 플레이어와 몬스터의 IsHeld에 대한 처리가 다른듯
void Actor::startRun()
{
	/* chase.c */
	// runto: Set a monster running after the hero

	addFlag(Actor::IsRunning);
	removeFlag(Actor::IsHeld);

	if (type != Actor::Hero)
	{
		// find_dest(): find the proper destination for the monster
		int prob = monsters[getChar() - L'A'].carry;

		// TODO: ...
	}
}

void Actor::potionHeal(bool extra)
{
	stats.hp += rollDice(stats.level, extra ? 8 : 4);

	if (stats.hp > stats.maxhp)
	{
		if (extra && stats.hp > stats.maxhp + stats.level + 1)
			++stats.maxhp;

		stats.hp = ++stats.maxhp;
	}
}

void Actor::engulf()
{
	takeDamage(VfHit);

	if (isDead())
		death(L"a venus flytrap");
}

void Actor::unhold()
{
	removeFlag(Actor::IsHeld);
	VfHit = 0;
	monsters[L'F' - L'A'].stats.damage = "000x0";
}

bool Actor::checkGroup(Item& item) const
{
	if (item.getGroup() == 0)
		return false;

	for (const auto& i : pack)
	{
		if (i->getGroup() == item.getGroup())
			return true;
	}

	return false;
}

Item* Actor::addPack(std::unique_ptr<Item> item)
{
	switch (item->getType())
	{
	case ItemType::Gold:
		purse += item->getCount();
		return nullptr;

	case ItemType::Potion:
	case ItemType::Scroll:
	case ItemType::Food:
		for (const auto& i : pack)
		{
			if (i->isSame(*item))
			{
				i->addCount(item->getCount());
				return i.get();
			}
		}
		break;

	case ItemType::Weapon:
		if (item->getGroup() > 0)
		{
			for (const auto& i : pack)
			{
				if (i->getGroup() == item->getGroup())
				{
					i->addCount(item->getCount());
					return i.get();
				}
			}
		}
		break;
	}

	// HACK: Repaint item
	if (hasFlag(Actor::IsHallucinating))
		item->repaint();

	pack.emplace_back(std::move(item));

	return pack.back().get();
}

std::unique_ptr<Item> Actor::unpack(Item& item, bool hurl)
{
	/* pack.c */
	// leave_pack: Take an item out of the pack

	auto found = std::find_if(pack.begin(), pack.end(), [&] (const auto& i) { return i.get() == &item; });
	assert(found != pack.end());

	Item* i = (*found).get();

	//
	if (thrownItem == i)
		thrownItem = nullptr;
	
	if (hurl)
	{
		if (i->getCount() > 1)
			thrownItem = i;
		else
			thrownItem = nullptr;
	}
	//

	if (i->getCount() > 1 && (i->getGroup() == 0 || hurl))
		return i->detachOne();

	auto result = std::move(*found);
	pack.erase(found);

	return result;
}

int Actor::getNumItems() const
{
	int count = 0;

	for (const auto& item : pack)
	{
		if (item->getGroup() == 0)
			count += item->getCount();
		else
			++count;
	}

	return count;
}

void Actor::setEquipment(Slot slot, Item* item)
{
	assert(!item || !equipment[slot]);

	equipment[slot] = item;
}

Item* Actor::getEquipment(Slot slot) const
{
	return equipment[slot];
}

bool Actor::isEquipment(Slot slot, int which) const
{
	return equipment[slot] && equipment[slot]->getWhich() == which;
}

bool Actor::hasRing(int ringType) const
{
	if (isEquipment(LeftRing, ringType))
		return true;
	if (isEquipment(RightRing, ringType))
		return true;

	return false;
}

void Actor::addEffect(Flags flag, int duration)
{
	for (const auto& effect : effects)
	{
		if (static_cast<Flags>(effect->getFlag()) == flag)
		{
			effect->lengthen(duration);
			return;
		}
	}

	auto effect = std::make_unique<Effect>(flag, duration);
	effect->start(*this);

	effects.emplace_back(std::move(effect));
}

void Actor::removeEffect(Flags flag)
{
	for (auto it = effects.begin(); it != effects.end(); )
	{
		if (static_cast<Flags>((*it)->getFlag()) == flag)
		{
			(*it)->end(*this);
			it = effects.erase(it);
		}

		else
			++it;
	}
}

void Actor::updateEffects()
{
	for (auto it = effects.begin(); it != effects.end(); )
	{
		(*it)->update(*this);

		if ((*it)->isFinished())
		{
			(*it)->end(*this);
			it = effects.erase(it);
		}

		else
			++it;
	}
}

int Actor::getEffectDuration(Flags flag) const
{
	for (const auto& effect : effects)
	{
		if (static_cast<Flags>(effect->getFlag()) == flag)
			return effect->getDuration();
	}

	return 0;
}

bool Actor::resistCheck(int which) const
{
	/* monsters.c */
	// save: See if he saves against various nasty things

	if (which == Actor::Magic)
	{
		if (isEquipment(Actor::LeftRing, Ring::Protect))
			which -= getEquipment(Actor::LeftRing)->getRingPower();
		if (isEquipment(Actor::RightRing, Ring::Protect))
			which -= getEquipment(Actor::RightRing)->getRingPower();
	}

	return throwCheck(which);
}

bool Actor::throwCheck(int which) const
{
	// save_throw: See if a creature save against something

	int need = 14 + which - stats.level / 2;

	return rollDice(1, 20) >= need;
}

bool Actor::levitCheck() const
{
	/* command.c */
	// levit_check: Check to see if she's levitating, and if she is, print an
	//              appropriate message.

	if (hasFlag(Actor::IsLevitating))
	{
		message(L"you can't.  You're floating off the ground!", Color::White);
		return true;
	}
	
	return false;
}

void Actor::rustArmor()
{
	/* move.c */
	// rust_armor: Rust the given armor, if it is a legal kind to rust, and we
	//             aren't wearing a magic ring.

	// TODO: 러스트 함정을 통해 아머 수치가 제대로 작동하는지 확인해볼 것

	Item* armor = equipment[Actor::Armor];

	if (!armor ||
		armor->isSame(ItemType::Armor, Armor::Leather) ||
		armor->getArmor() >= 9)
		return;

	if (armor->hasFlag(Item::IsProtected) || hasRing(Ring::MaintainArmor))
		message(L"the rust vanishes instantly.", Color::White);

	else
	{
		armor->degrade();
		message(L"your armor appears to be weaker now. Oh my!", Color::Red);
	}
}

void Actor::repaint()
{
	disguise = getChar();

	if (getChar() == L'@')
		setColor(Color::White);
	else
		setColor(monsters[getChar() - L'A'].color);
}

void Actor::draw(Console& console)
{
	console.setChar(getPosition().x, getPosition().y, disguise, getColor());
}

Actor::Ptr Actor::createMonster(Rng& rng, int depth, bool wander)
{
	// randmonster: Pick a monster to show up

	int d;
	const auto& list = wander ? wanderers : dwellers;

	do
	{
		d = depth + (rng.getInt(10) - 6);

		if (d < 0)
			d = rng.getInt(5);
		if (d > 25)
			d = rng.getInt(5) + 21;
	} while (list[d] == 0);

	int type = list[d];

	return createMonster(type, depth);
}

Actor::Ptr Actor::createMonster(int depth, bool wander)
{
	// randmonster: Pick a monster to show up

	int d;
	const auto& list = wander ? wanderers : dwellers;

	do
	{
		d = depth + (randomInt(10) - 6);

		if (d < 0)
			d = randomInt(5);
		if (d > 25)
			d = randomInt(5) + 21;
	} while (list[d] == 0);

	int type = list[d];

	return createMonster(type, depth);
}

Actor::Ptr Actor::createMonster(int type, int depth)
{
	const Monster* mp = &monsters[type - L'A'];
	int levelAdd = depth - AmuletLevel;

	if (levelAdd < 0)
		levelAdd = 0;

	auto monster = std::make_unique<Actor>(type);

	monster->type = Actor::Enemy;
	monster->setColor(mp->color);

	monster->stats.level = mp->stats.level + levelAdd;
	monster->stats.maxhp = monster->stats.hp = rollDice(monster->stats.level, 8);
	monster->stats.armor = mp->stats.armor - levelAdd;
	monster->stats.damage = mp->stats.damage;
	monster->stats.str = mp->stats.str;

	/* monsters.c */
	// exp_add: Experience to add for this monster's level/hit points
	int expAdd = monster->stats.maxhp;

	if (monster->stats.level == 1)
		expAdd /= 8;
	else
		expAdd /= 6;

	if (monster->stats.level > 9)
		expAdd *= 20;
	else if (monster->stats.level > 6)
		expAdd *= 4;

	monster->stats.exp = mp->stats.exp + levelAdd * 10 + expAdd;
	monster->flags = mp->flags;

	if (depth > 29)
		monster->addFlag(Actor::IsHasted);

	if (type == L'X')
	{
		monster->disguise = Item::getRandomChar();
		// setColor(ItemColor);
	}

	return monster;
}

std::wstring Actor::chooseStr(const std::wstring& str1, const std::wstring& str2)
{
	/* misc.c */
	// choose_str: Choose the first or second string depending on whether it the
	//             player is tripping
	
	return hasFlag(Actor::IsHallucinating) ? str1 : str2;
}
