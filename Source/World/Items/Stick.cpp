#include "Stick.hpp"
#include "Potion.hpp"
#include "Weapon.hpp"
#include "../../World/World.hpp"
#include "../../Graphics/Color.hpp"
#include "../../Utility/Utility.hpp"
#include "../../States/ThrowOrZap.hpp"

#include <sstream>

namespace
{
	/* extern.c */
	std::vector<Item::Info> sticks =
	{
		{ L"light",         12, 250 },
		{ L"invisibility",   6,   5 },
		{ L"lightning",      3, 330 },
		{ L"fire",           3, 330 },
		{ L"cold",           3, 330 },
		{ L"polymorph",     15, 310 },
		{ L"magic missile", 10, 170 },
		{ L"haste monster", 10,   5 },
		{ L"slow monster",  11, 350 },
		{ L"drain life",     9, 300 },
		{ L"nothing",        1,   5 },
		{ L"teleport away",  6, 340 },
		{ L"teleport to",    6,  50 },
		{ L"cancellation",   5, 280 },
	};

	/* init.c */
	std::vector<std::wstring> wood =
	{
		L"avocado wood",
		L"balsa",
		L"bamboo",
		L"banyan",
		L"birch",
		L"cedar",
		L"cherry",
		L"cinnibar",
		L"cypress",
		L"dogwood",
		L"driftwood",
		L"ebony",
		L"elm",
		L"eucalyptus",
		L"fall",
		L"hemlock",
		L"holly",
		L"ironwood",
		L"kukui wood",
		L"mahogany",
		L"manzanita",
		L"maple",
		L"oaken",
		L"persimmon wood",
		L"pecan",
		L"pine",
		L"poplar",
		L"redwood",
		L"rosewood",
		L"spruce",
		L"teak",
		L"walnut",
		L"zebrawood",
	};

	std::vector<std::wstring> metal =
	{
		L"aluminum",
		L"beryllium",
		L"bone",
		L"brass",
		L"bronze",
		L"copper",
		L"electrum",
		L"gold",
		L"iron",
		L"lead",
		L"magnesium",
		L"mercury",
		L"nickel",
		L"pewter",
		L"platinum",
		L"steel",
		L"silver",
		L"silicon",
		L"tin",
		L"titanium",
		L"tungsten",
		L"zinc",
	};

	std::vector<std::wstring> types;
	std::vector<std::wstring> names;
}

std::wstring Stick::getAName() const
{
	std::wostringstream oss;

	if (sticks[which].know)
		oss << L"a " << types[which] << L" of " << sticks[which].name;
	else
		oss << (isVowel(names[which]) ? L"an " : L"a ") << names[which] << L" " << types[which];

	/* sticks.c */
	// charge_str: Return an appropriate string for a wand charge
	if (hasFlag(Item::IsKnow))
		oss << L" [" << charges << L"]";

	return oss.str();
}

std::string Stick::getDamage() const
{
	if (types[which] == L"staff")
		return "2x3";

	// wand
	return  "1x1";
}

std::string Stick::getDamageThrown() const
{
	return "1x1";
}

bool Stick::use(Actor& actor)
{
	/* sticks.c */
	// do_zap: Perform a zap with a wand

	ThrowOrZap::Callback callback = [] (Direction dir) { return nullptr; };

	message(L"which direction?", Color::Blue);

	if (charges == 0)
	{
		callback = [&] (Direction dir)
		{
			message(L"nothing happens.", Color::White);
			
			return nullptr;
		};

		pushState(std::make_unique<ThrowOrZap>(std::move(callback)));

		return true;
	}

	--charges;

	Level& level = getLevel();

	switch (which)
	{
	case Stick::Light:
		sticks[which].know = true;
		callback = [&] (Direction dir)
		{
			playSound(SoundID::Zap);

			if (level.lightRoom(actor.getPosition()))
			{
				// proom->r_flags &= ~ISDARK;
				// enter_room(&hero);
				message(L"the room is lit by a shimmering " + Potion::pickColor(actor, L"blue") + L" light.", Color::White);
			}

			else // Corridor
				message(L"the corridor glows and then fades.", Color::White);

			return nullptr;
		};
		break;

	case Stick::Invisibility:
	case Stick::Polymorph:
	case Stick::TeleportAway:
	case Stick::TeleportTo:
	case Stick::Cancel:
		callback = [&] (Direction dir)
		{
			playSound(SoundID::Zap);

			sf::Vector2i pos = actor.getPosition();
			Actor* monster = nullptr;

			do
			{
				pos += dir;

				if (!level.at(pos).passable)
					break;

				monster = level.actorAt(pos);

			} while (!monster);

			if (monster)
			{
				// UNDONE: Invis/TeleportTo는 제외해야 하지 않을까?
				if (monster->getChar() == L'F')
					actor.unhold(); // actor.removeFlag(Actor::IsHeld);

				switch (which)
				{
				case Stick::Invisibility:
					monster->addFlag(Actor::IsInvisible);
					break;

				case Stick::Polymorph:
				{
					// UNDONE:
					level.detach(*monster);

					auto newMonster = Actor::createMonster(L'A' + randomInt(26), getWorld().getDepth());
					newMonster->setPosition(pos);

					if (getWorld().isVisible(*newMonster))
						sticks[which].know = true;

					level.attach(std::move(newMonster));
				}
				break;

				case Stick::TeleportAway:
					teleport(*monster);
					monster->addFlag(Actor::IsRunning);
					playSound(SoundID::Teleport);
					break;

				case Stick::TeleportTo:
					monster->setPosition(pos + dir);
					monster->addFlag(Actor::IsRunning);
					break;

				case Stick::Cancel:
					// TODO: IsCancelled 플래그를 삭제하고 몬스터의 스킬 플래그를 리셋시키는 방식으로 변경
					monster->addFlag(Actor::IsCancelled);
					monster->removeFlag(Actor::IsInvisible);
					monster->removeFlag(Actor::IsConfused); // Fix: CanConfuse(CanHuh) > IsConfused(IsHuh)
					// TODO: 원래 모습으로 되돌림 (미믹)
					// monster->disguise = monster->type;
					break;
				}
			}

			return nullptr;
		};
		break;

	case Stick::Lightning:
	case Stick::Fire:
	case Stick::Cold:
		sticks[which].know = true;
		callback = [&] (Direction dir)
		{
			playSound(SoundID::Zap);

			wchar_t ch;

			if (dir.y == 0)
				ch = L'-';
			else if (dir.x == 0)
				ch = L'|';
			else if (dir.x * dir.y < 0)
				ch = L'/';
			else if (dir.x * dir.y > 0)
				ch = L'\\';

			sf::Color color;
			std::wstring name;

			switch (which)
			{
			case Stick::Lightning:
				color = sf::Color::White;
				name = L"bolt";
				break;

			case Stick::Fire:
				color = Color::Red;
				name = L"flame";
				playSound(SoundID::Fire);
				break;

			case Stick::Cold:
				color = Color::Blue;
				name = L"ice";
				playSound(SoundID::Ice);
				break;
			}

			auto bolt = Weapon::createBolt(ch, color, name, "6x6", 100, 0);
			bolt->setPosition(actor.getPosition());

			return bolt;
		};
		break;

	case Stick::Missile:
		sticks[which].know = true;
		callback = [&] (Direction dir)
		{
			playSound(SoundID::Zap);

			auto missile = Weapon::createBolt(L'*', Color::Cyan, L"missile", "1x4", 100, 1, true);
			missile->setPosition(actor.getPosition());

			return missile;
		};
		break;

	case Stick::Haste:
	case Stick::Slow:
		callback = [&] (Direction dir)
		{
			playSound(SoundID::Zap);

			sf::Vector2i pos = actor.getPosition();
			Actor* monster = nullptr;

			do
			{
				pos += dir;

				if (!level.at(pos).passable)
					break;

				monster = level.actorAt(pos);

			} while (!monster);

			if (monster)
			{
				switch (which)
				{
				case Stick::Haste:
					if (monster->hasFlag(Actor::IsSlowed))
						monster->removeFlag(Actor::IsSlowed);
					else
						monster->addFlag(Actor::IsHasted);
					break;

				case Stick::Slow:
					if (monster->hasFlag(Actor::IsHasted))
						monster->removeFlag(Actor::IsHasted);
					else
						monster->addFlag(Actor::IsSlowed);
					monster->turn = true;
					break;
				}

				monster->startRun();
			}

			return nullptr;
		};
		break;

	case Stick::Drain:
		callback = [&] (Direction dir)
		{
			playSound(SoundID::Zap);

			if (actor.getHp() < 2)
				message(L"you are too weak to use it.", Color::White);

			else
			{
				/* sticks.c */
				// drain: Do drain hit points from player shtick

				std::vector<Actor*> monsters;

				// Get visible monsters
				level.foreachActors([&] (Actor& monster)
				{
					if (monster.getPosition() != actor.getPosition() && level.at(monster.getPosition()).visible)
						monsters.emplace_back(&monster);
				});

				if (monsters.empty())
					message(L"you have a tingling feeling.", Color::White);

				else
				{
					int damage = actor.getHp() / 2;

					actor.takeDamage(damage);

					damage /= monsters.size();

					if (damage < 0)
						damage = 1;

					for (const auto& monster : monsters)
					{
						monster->takeDamage(damage);

						if (monster->isDead())
							actor.killed(*monster);
						else
							monster->startRun();
					}
				}
			}

			return nullptr;
		};
		break;
	}

	pushState(std::make_unique<ThrowOrZap>(std::move(callback)));

	return true;
}

void Stick::identify()
{
	sticks[which].know = true;
	addFlag(Item::IsKnow);
}

void Stick::initMaterials(Rng& rng)
{
	/* init.c */
	// init_materials: Initialize the construction materials for wands and staffs

	rng.shuffle(wood);
	rng.shuffle(metal);

	types.resize(sticks.size());
	names.resize(sticks.size());

	for (std::size_t i = 0; i < sticks.size(); ++i)
	{
		if (rng.getBool())
		{
			types[i] = L"wand";
			names[i] = metal[i];
		}

		else
		{
			types[i] = L"staff";
			names[i] = wood[i];
		}
	}
}

Item::Ptr Stick::createStick(int which, int charges)
{
	/* sticks.c */
	// fix_stick: Set up a new stick

	auto stick = std::make_unique<Stick>(ItemType::Stick);
	stick->which = which;

	if (charges == 0)
	{
		if (which == Stick::Light)
			charges = randomInt(10) + 10;
		else
			charges = randomInt(5) + 3;
	}

	stick->charges = charges;

	return stick;
}

int Stick::pickOne(Rng& rng)
{
	return Item::pickOne(rng, sticks);
}
