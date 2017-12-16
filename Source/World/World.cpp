#include "World.hpp"
#include "ActorPlayer.hpp"
#include "../Game/Console.hpp"
#include "../Game/Status.hpp"
#include "../Graphics/Color.hpp"
#include "../Utility/Utility.hpp"
#include "../Action/WalkAction.hpp"
#include "../Action/MonsterAction.hpp"
#include "../States/Help.hpp"
#include "../States/Inventory.hpp"

#include "Items/Potion.hpp"
#include "Items/Scroll.hpp"
#include "Items/Weapon.hpp"
#include "Items/Armor.hpp"
#include "Items/Ring.hpp"
#include "Items/Stick.hpp"

#include <SFML/Window/Event.hpp>

World::World(Console& console)
	: console(console)
	, fov(console)
{
	GameObject::setWorld(*this);
	
	// rng.setSeed(0x0);

	sf::Vector2i mapSize = console.getSize();

	auto level = std::make_unique<Level>(rng, mapSize.x, mapSize.y);
	currentLevel = level.get();
	levels.emplace_back(std::move(level));

	Item::initItems(rng);

	auto actor = ActorPlayer::createActor();
	playerActor = actor.get();
	currentLevel->attach(std::move(actor));

	dungeonGenerator.generate(*currentLevel, rng, currentDepth);
	playerActor->setPosition(currentLevel->upStairs);

	resetFovRange();
	fov.load(*currentLevel);
	computeFov();

	updateStatus();

	Word you(L"adventurer", Color::White);

	message(L"hello, {0}.  Welcome to the Dungeon of Doom.", { you });
	message(L"(Press F1 or ? for help.)", Color::DarkGray);
}

#include "../Action/ItemAction.hpp"

void World::handleKeyboard(sf::Keyboard::Key key)
{
	// if (playerActor->hasFlag(Actor::NoCommand))
		// return;

	switch (key)
	{
#ifdef _DEBUG
	case sf::Keyboard::LBracket:
		if (!playerActor->isDead() && currentDepth > 1)
		{
			ascend();
			computeFov();
			updateStatus();
		}
		break;

	case sf::Keyboard::RBracket:
		if (!playerActor->isDead())
		{
			descend();
			computeFov();
			updateStatus();
		}
		break;

	case sf::Keyboard::F2:
		debugDraw = !debugDraw;
		break;

	case sf::Keyboard::F3:
		for (auto& item : playerActor->pack)
			item->identify();
		break;

	case sf::Keyboard::F4:
		if (playerActor->isDead())
		{
			playerActor->revive();
			updateStatus();
			message(L"you revived.", Color::Blue);
		}
		break;

	case sf::Keyboard::Num1:
		for (int i = 0; i < Potion::MaxPotions; ++i)
		{
			auto item = Item::createItem(ItemType::Potion, i);
			item->identify();
			item->addFlag(Item::IsFound);
			playerActor->addPack(std::move(item));
		}
		break;

	case sf::Keyboard::Num2:
		for (int i = 0; i < Scroll::MaxScrolls; ++i)
		{
			auto item = Item::createItem(ItemType::Scroll, i);
			item->identify();
			item->addFlag(Item::IsFound);
			playerActor->addPack(std::move(item));
		}
		break;

	case sf::Keyboard::Num3:
		for (int i = 0; i < Weapon::MaxWeapons; ++i)
		{
			auto item = Item::createItem(ItemType::Weapon, i);
			item->identify();
			item->addFlag(Item::IsFound);
			playerActor->addPack(std::move(item));
		}
		break;

	case sf::Keyboard::Num4:
		for (int i = 0; i < Armor::MaxArmors; ++i)
		{
			auto item = Item::createItem(ItemType::Armor, i);
			item->identify();
			item->addFlag(Item::IsFound);
			playerActor->addPack(std::move(item));
		}
		break;

	case sf::Keyboard::Num5:
		for (int i = 0; i < Ring::MaxRings; ++i)
		{
			auto item = Item::createItem(ItemType::Ring, i);
			item->identify();
			item->enchant();
			item->addFlag(Item::IsFound);
			playerActor->addPack(std::move(item));
		}
		break;

	case sf::Keyboard::Num6:
		for (int i = 0; i < Stick::MaxSticks; ++i)
		{
			auto item = Item::createItem(ItemType::Stick, i);
			item->identify();
			item->addFlag(Item::IsFound);
			playerActor->addPack(std::move(item));
		}
		break;
#endif

	default:
		if (!playerActor->isDead())
		{
			auto action = player.getAction(key);

			if (action)
			{
				// NOTE: typeid, typeid().name, dynamic_cast() != nullptr
				if (typeid(*action) == typeid(RunAction))
				{
					playerActor->setRunning(true);
					playerActor->lastDir = dynamic_cast<RunAction*>(action.get())->dir;
				}

				else
				{
					if (action->perform(*playerActor))
						endPlayerTurn();
				}
			}
		}
		break;
	}
}

void World::update(sf::Time dt)
{
	// TODO: 죽었을때 상태이상 해제

	if (!playerActor->isDead())
	{
		if (playerActor->hasFlag(Actor::NoCommand))
		{
			const sf::Time secondsPerTick = sf::seconds(0.25f);
			static sf::Time elapsedTime;

			elapsedTime += dt;

			if (elapsedTime >= secondsPerTick)
			{
				// elapsedTime -= secondsPerTick;
				elapsedTime = sf::Time::Zero;
				playSound(SoundID::Tick);

				endPlayerTurn();
			}
		}

		else if (playerActor->isRunning())
			playerActor->run();
	}

	currentLevel->update(dt);

	removeStoppedSounds();
}

void World::draw()
{
	console.clear();

	if (debugDraw)
		currentLevel->debugDraw(console);

	else
	{
		currentLevel->draw(console);
		fov.draw();
		currentLevel->postProcess(console);
	}

	if (playerActor->hasFlag(Actor::NoCommand))
	{
		int size = 3 - playerActor->getEffectDuration(Actor::NoCommand) % 4;

		// StatusWidth 20
		for (int i = 0; i < size; ++i)
			console.setChar(21 + i, console.getSize().y - 1, 0x00b7, sf::Color::White);
	}
}

int World::getDepth() const
{
	return currentDepth;
}

Level& World::getLevel() const
{
	return *currentLevel;
}

ActorPlayer& World::getPlayerActor() const
{
	return *playerActor;
}

bool World::isVisible(const Actor& actor) const
{
	if (&actor == playerActor)
		return true;

	if (playerActor->hasFlag(Actor::IsBlind))
		return false;

	if (!currentLevel->at(actor.getPosition()).visible)
		return false;

	if (!actor.hasFlag(Actor::IsInvisible))
		return true;

	return playerActor->hasFlag(Actor::CanSee)
		|| playerActor->hasRing(Ring::SeeInvisible);
}

bool World::isVisible(const Item& item) const
{
	if (playerActor->hasFlag(Actor::IsBlind))
		return false;

	return currentLevel->at(item.getPosition()).visible;
}

void World::ascend()
{
	// TODO: Create the exit if you're at the first level and have the amulet

	if (playerActor->hasFlag(Actor::IsHallucinating))
		currentLevel->repaint();

	auto actor = currentLevel->detach(*playerActor);
	currentLevel = levels[--currentDepth - 1].get();
	currentLevel->attach(std::move(actor));

	playerActor->setPosition(currentLevel->downStairs);

	fov.load(*currentLevel);

	if (playerActor->hasFlag(Actor::IsHallucinating))
		currentLevel->colorize();

	playSound(SoundID::Stairs);
}

void World::descend()
{
	if (playerActor->hasFlag(Actor::IsHallucinating))
		currentLevel->repaint();

	auto actor = currentLevel->detach(*playerActor);

	if (++currentDepth > levels.size())
	{
		sf::Vector2i mapSize = console.getSize();
		auto newLevel = std::make_unique<Level>(rng, mapSize.x, mapSize.y);
		currentLevel = newLevel.get();
		levels.emplace_back(std::move(newLevel));
		dungeonGenerator.generate(*currentLevel, rng, currentDepth);
	}

	else
		currentLevel = levels[currentDepth - 1].get();
	
	currentLevel->attach(std::move(actor));

	playerActor->setPosition(currentLevel->upStairs);

	fov.load(*currentLevel);

	if (playerActor->hasFlag(Actor::IsHallucinating))
		currentLevel->colorize();

	playSound(SoundID::Stairs);
}

void World::aggravate()
{
	/* misc.c */
	// aggravate: Aggravate all the monsters on this level

	currentLevel->foreachActors([] (Actor& monster)
	{
		if (monster.getType() == Actor::Enemy)
			monster.startRun();
	});
}

void World::endPlayerTurn()
{
	computeFov();

	if (--playerActor->ntimes <= 0)
	{
		playerActor->ntimes = 1;
		playerActor->updateEffects();
		playerActor->digest();
		playerActor->rest();

		updateEnemies();
	}

	updateStatus();
	currentLevel->removeDeadActors();

	if (playerActor->hasFlag(Actor::IsHallucinating))
		currentLevel->colorize();

	if (playerActor->isDead())
	{
		playerActor->setDisguise(L'%');
		playerActor->setColor(Color::Blood);

		return;
	}

	/* monster.c */
	// wanderer: Create a new wandering monster and aim it at the player
	static int wanderTime = spread(70);

	if (--wanderTime <= 0)
	{
		wanderTime = spread(70);

		sf::Vector2i pos;

		do
			pos = currentLevel->findPassableTile();
		while (currentLevel->at(pos).visible);

		auto monster = Actor::createMonster(currentDepth, true);
		monster->setPosition(pos);
		monster->startRun();
		currentLevel->attach(std::move(monster));
	}

	// TODO: haste 상태일때 반지의 발동 처리

	if (playerActor->isEquipment(Actor::LeftRing, Ring::Search))
		; // playerActor->search();

	else if (playerActor->isEquipment(Actor::LeftRing, Ring::Teleport) && randomInt(50) == 0)
	{
		teleport(*playerActor);
		computeFov();
	}

	if (playerActor->isEquipment(Actor::RightRing, Ring::Search))
		; // playerActor->search();

	else if (playerActor->isEquipment(Actor::RightRing, Ring::Teleport) && randomInt(50) == 0)
	{
		teleport(*playerActor);
		computeFov();
	}
}

void World::updateEnemies()
{
	/* chase.c */
	// runners: Make all the running monsters move

	for (auto& monster : currentLevel->actors)
	{
		// Stop processing turn
		if (playerActor->isDead())
			break;

		if (monster.get() == playerActor || monster->isDead())
			continue;

		std::make_unique<MonsterAction>()->perform(*monster);

		// TODO: do_chase: stoprun(~ISRUN)
		// TODO: wake_monster(look, door_open)
		// TODO: seen_stairs?

		monster->updateEffects();
	}
}

void World::updateStatus()
{
	getStatus().setState(playerActor->getHungryState(), sf::Color::White);
	getStatus().setStats(*playerActor);
	getStatus().setDepth(currentDepth);
}

void World::computeFov()
{
	fov.compute(sf::Vector2f(playerActor->getPosition()), fovRange);
}

void World::reloadFov()
{
	fov.load(*currentLevel);
}

void World::setFovRange(float range)
{
	fovRange = range;
}

float World::getFovRange() const
{
	return fovRange;
}

void World::resetFovRange()
{
	fovRange = 10.f;
}
