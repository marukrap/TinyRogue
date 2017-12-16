#include "ActorPlayer.hpp"
#include "World.hpp"
#include "../Graphics/Color.hpp"
#include "../Utility/Utility.hpp"
#include "../Action/WalkAction.hpp"

// Items
#include "Items/Potion.hpp"
#include "Items/Scroll.hpp"
#include "Items/Weapon.hpp"
#include "Items/Armor.hpp"
#include "Items/Ring.hpp"
#include "Items/Stick.hpp"

#include <iostream>
#include <algorithm> // min, max

namespace
{
	const int HungerTime	= 1300;
	const int MoreTime		= 150;
	const int StomachSize	= 2000;
	const int StarveTime	= 850;

	const Actor::Stats InitStats = { 0, 1, 10, "1x4", 12, 12, 16, 16 };
}

void ActorPlayer::attack(Actor& target)
{
	running = false;
	quiet = 0;

	Actor::attack(target);
}

void ActorPlayer::takeDamage(int points)
{
	running = false;
	quiet = 0;

	Actor::takeDamage(points);
}

std::wstring ActorPlayer::getHungryState() const
{
	if (foodLeft < 0)
		return L"Faint";	// 3
	else if (foodLeft < 150)
		return L"Weak";		// 2
	else if (foodLeft < 300)
		return L"Hungry";	// 1
	else
		return L"";			// 0
}

void ActorPlayer::eatFood()
{
	if (foodLeft < 0)
		foodLeft = 0;

	foodLeft += HungerTime - 200 + randomInt(400);

	if (foodLeft > StomachSize)
		foodLeft = StomachSize;

#ifdef _DEBUG
	std::cout << "Food Left: " << foodLeft << '\n';
#endif
}

void ActorPlayer::setRunning(bool flag)
{
	running = flag;
}

bool ActorPlayer::isRunning() const
{
	return running;
}

void ActorPlayer::run()
{
	auto action = std::make_unique<RunAction>(lastDir);

	if (action->perform(*this))
	{
		getWorld().endPlayerTurn();

		// Stop if you notice a monster
		Level& level = getWorld().getLevel();
		sf::Vector2i pos = getPosition();
		int range = static_cast<int>(getWorld().getFovRange());

		int left = std::max(0, pos.x - range);
		int top = std::max(0, pos.y - range);
		int right = std::min(level.width - 1, pos.x + range);
		int bottom = std::min(level.height - 1, pos.y + range);

		// TODO: Reduce actorAt calls
		for (int y = top; y <= bottom; ++y)
			for (int x = left; x <= right; ++x)
			{
				if (x == pos.x && y == pos.y)
					continue;

				if (level.at(x, y).visible && level.actorAt({ x, y }))
				{
					running = false;
					return;
				}
			}
	}

	else
		running = false;
}

void ActorPlayer::revive()
{
	setDisguise(getChar());
	setColor(Color::White);

	stats.hp = stats.maxhp;
	foodLeft = HungerTime;

	// remove effects
	flags = 0;
	effects.clear();
}

void ActorPlayer::digest()
{
	/* daemons.c */
	// stomach: Digest the hero's food

	auto oldState = getHungryState();

	if (foodLeft <= 0)
	{
		if (foodLeft-- < -StarveTime)
		{
			message(L"you died by starvation.", Color::Blood);
			kill();
		}

		else if (!hasFlag(Actor::NoCommand) && randomInt(5) == 0)
		{
			addEffect(Actor::NoCommand, randomInt(8) + 4);

			message(chooseStr(L"the munchies overpower your motor capabilities.  You freak out.",
							  L"you feel too weak from lack of food.  You faint."), Color::White);
		}
	}

	else
	{
		int oldFood = foodLeft;

		if (getEquipment(Actor::LeftRing))
			foodLeft -= getEquipment(Actor::LeftRing)->ringEat();
		if (getEquipment(Actor::RightRing))
			foodLeft -= getEquipment(Actor::RightRing)->ringEat();
		
		foodLeft -= 1;
		foodLeft -= (amulet ? 1 : 0);

		if (foodLeft < MoreTime && oldFood >= MoreTime)
			message(chooseStr(L"the munchies are interfering with your motor capabilites.",
							  L"you are starting to feel weak."), Color::White);
		
		else if (foodLeft < MoreTime * 2 && oldFood >= MoreTime * 2)
			message(chooseStr(L"you are getting the munchies.",
							  L"you are starting to get hungry."), Color::White);
	}

	if (getHungryState() != oldState)
	{
		removeFlag(Actor::IsRunning);
		running = false;
	}
}

void ActorPlayer::rest()
{
	/* daemons.c */
	// doctor: A healing daemon that restore hit points after rest

	int level = stats.level;
	int oldHp = stats.hp;

	++quiet;

	if (level < 8)
	{
		if (quiet + (level << 1) > 20) // level * 2 ?
			++stats.hp;
	}

	else
	{
		if (quiet >= 3)
			stats.hp += randomInt(level - 7) + 1;
	}

	if (isEquipment(Actor::LeftRing, Ring::Regenerate))
		++stats.hp;
	if (isEquipment(Actor::RightRing, Ring::Regenerate))
		++stats.hp;

	if (oldHp != stats.hp)
	{
		if (stats.hp > stats.maxhp)
			stats.hp = stats.maxhp;

		quiet = 0;
	}
	
	// TODO: 플레이어가 공격을 하거나, 당하는 경우 quiet = 0
}

/* init.c */
// init_player: Roll her up
ActorPlayer::Ptr ActorPlayer::createActor()
{
	auto actor = std::make_unique<ActorPlayer>(L'@');
	
	actor->type = Actor::Hero;
	actor->setColor(Color::White);
	actor->addFlag(Actor::IsRunning); // REMOVE: Remove this, just added for test

	actor->stats = InitStats;
	actor->foodLeft = HungerTime;

	auto food = Item::createItem(ItemType::Food, 0);
	food->addFlag(Item::IsFound);
	actor->addPack(std::move(food));

	// +1 ring mail
	auto armor = Armor::createArmor(Armor::RingMail, 1);
	armor->addFlag(Item::IsKnow);
	armor->addFlag(Item::IsFound);
	actor->setEquipment(Actor::Armor, armor.get());
	actor->addPack(std::move(armor));

	// A +1,+1 mace
	auto weapon = Weapon::createWeapon(Weapon::Mace, 1, 1);
	weapon->addFlag(Item::IsKnow);
	weapon->addFlag(Item::IsFound);
	actor->setEquipment(Actor::Weapon, weapon.get());
	actor->addPack(std::move(weapon));

	// A +1 bow
	auto bow = Weapon::createWeapon(Weapon::Bow, 1, 0);
	bow->addFlag(Item::IsKnow);
	bow->addFlag(Item::IsFound);
	actor->addPack(std::move(bow));

	// Some arrows
	auto arrows = Weapon::createWeapon(Weapon::Arrow);
	arrows->setCount(randomInt(15) + 25);
	arrows->addFlag(Item::IsKnow);
	arrows->addFlag(Item::IsFound);
	actor->addPack(std::move(arrows));

	return actor;
}
