#include "Scroll.hpp"
#include "Potion.hpp" // pickColor
#include "Weapon.hpp"
#include "../../World/World.hpp"
#include "../../Graphics/Color.hpp"
#include "../../Utility/Utility.hpp"
#include "../../States/Inventory.hpp"

#include <sstream>

namespace
{
	/* extern.c */
	std::vector<Item::Info> scrolls =
	{
		{ L"monster confusion",             7, 140 },
		{ L"magic mapping",                 4, 150 },
		{ L"hold monster",                  2, 180 },
		{ L"sleep",                         3,   5 },
		{ L"enchant armor",                 7, 160 },
		{ L"identify potion",              10,  80 },
		{ L"identify scroll",              10,  80 },
		{ L"identify weapon",               6,  80 },
		{ L"identify armor",                7, 100 },
		{ L"identify ring, wand or staff", 10, 115 },
		{ L"scare monster",                 3, 200 },
		{ L"food detection",                2,  60 },
		{ L"teleportation",                 5, 165 },
		{ L"enchant weapon",                8, 150 },
		{ L"create monster",                4,  75 },
		{ L"remove curse",                  7, 105 },
		{ L"aggravate monsters",            3,  20 },
		{ L"protect armor",                 2, 250 },
	};

	/* init.c */
	const std::vector<std::wstring> sylls =
	{
		L"a", L"ab", L"ag", L"aks", L"ala", L"an", L"app", L"arg", L"arze", L"ash",
		L"bek", L"bie", L"bit", L"bjor", L"blu", L"bot", L"bu", L"byt", L"comp",
		L"con", L"cos", L"cre", L"dalf", L"dan", L"den", L"do", L"e", L"eep", L"el",
		L"eng", L"er", L"ere", L"erk", L"esh", L"evs", L"fa", L"fid", L"fri", L"fu",
		L"gan", L"gar", L"glen", L"gop", L"gre", L"ha", L"hyd", L"i", L"ing", L"ip",
		L"ish", L"it", L"ite", L"iv", L"jo", L"kho", L"kli", L"klis", L"la", L"lech",
		L"mar", L"me", L"mi", L"mic", L"mik", L"mon", L"mung", L"mur", L"nej",
		L"nelg", L"nep", L"ner", L"nes", L"nes", L"nih", L"nin", L"o", L"od", L"ood",
		L"org", L"orn", L"ox", L"oxy", L"pay", L"ple", L"plu", L"po", L"pot",
		L"prok", L"re", L"rea", L"rhov", L"ri", L"ro", L"rog", L"rok", L"rol", L"sa",
		L"san", L"sat", L"sef", L"seh", L"shu", L"ski", L"sna", L"sne", L"snik",
		L"sno", L"so", L"sol", L"sri", L"sta", L"sun", L"ta", L"tab", L"tem",
		L"ther", L"ti", L"tox", L"trol", L"tue", L"turs", L"u", L"ulk", L"um", L"un",
		L"uni", L"ur", L"val", L"viv", L"vly", L"vom", L"wah", L"wed", L"werg",
		L"wex", L"whon", L"wun", L"xo", L"y", L"yot", L"yu", L"zant", L"zeb", L"zim",
		L"zok", L"zon", L"zum",
	};

	const int MaxNames = 20; // MAXNAMES 40 > 20

	std::vector<std::wstring> names;
}

std::wstring Scroll::getAName() const
{
	std::wostringstream oss;

	if (count == 1)
		oss << L"a scroll";
	else
		oss << count << L" scrolls";

	if (scrolls[which].know)
		oss << L" of " + scrolls[which].name;
	else
		oss << L" titled '" << names[which] << L"'";

	return oss.str();
}

bool Scroll::use(Actor& actor)
{
	--count;

	Level& level = getLevel();

	switch (which)
	{
	case Scroll::Confuse:
		actor.addFlag(Actor::CanConfuse);
		message(L"your hands begin to glow " + Potion::pickColor(actor, L"red") + L".", Color::White);
		break;

	case Scroll::Map:
		scrolls[which].know = true;
		message(L"oh, now this scroll has a map on it.", Color::White);
		level.clearExplored(true);
		break;

	case Scroll::Hold:
	{
		int monsters = 0;

		for (int dy = -2; dy <= 2; ++dy)
			for (int dx = -2; dx <= 2; ++dx)
			{
				if (dx == 0 && dy == 0)
					continue;

				sf::Vector2i pos = actor.getPosition() + sf::Vector2i(dx, dy);

				if (!level.isInBounds(pos))
					continue;

				// TODO: actorAt을 여러번 호출하지 말고 foreachActors 사용
				Actor* monster = level.actorAt(pos);

				if (monster && monster->hasFlag(Actor::IsRunning))
				{
					monster->removeFlag(Actor::IsRunning);
					monster->addFlag(Actor::IsHeld);
					++monsters;
				}
			}

		if (monsters > 0)
		{
			scrolls[which].know = true;

			if (monsters > 1)
				message(L"the monsters around you freeze.", Color::White);
			else
				message(L"the monster freezes.", Color::White);
		}

		else
			message(L"you feel a strange sense of loss.", Color::White);
	}
		break;

	case Scroll::Sleep:
		scrolls[which].know = true;
		actor.addEffect(Actor::NoCommand, randomInt(spread(5)) + 4); // SLEEPTIME spread(5)
		message(L"you fall asleep.", Color::White);
		break;

	case Scroll::EnchantArmor:
		if (Item* armor = actor.getEquipment(Actor::Armor))
		{
			armor->enchant();
			armor->removeFlag(Item::IsCursed);
			message(L"your armor glows " + Potion::pickColor(actor, L"silver") + L" for a moment.", Color::White);
		}

		else
			message(L"you feel a strange sense of loss.", Color::White);
		break;

	case Scroll::IdPotion:
	case Scroll::IdScroll:
	case Scroll::IdWeapon:
	case Scroll::IdArmor:
	case Scroll::IdRingOrStick:
		scrolls[which].know = true;
		message(L"this scroll is an " + scrolls[which].name + L" scroll.", Color::White);
		pushState(std::make_unique<Inventory>(actor, static_cast<Inventory::Mode>(Inventory::IdPotion + which - static_cast<int>(Scroll::IdPotion))));
		return false; // Do not take a turn here

	case Scroll::Scare:
		message(L"you hear maniacal laughter in the distance.", Color::White);
		break;

	case Scroll::FoodFind:
	{
		bool found = false;

		level.foreachItems([&found] (Item& item)
		{
			if (item.getType() == ItemType::Food)
			{
				item.addFlag(Item::IsDetected);
				found = true;
			}
		});

		if (found)
		{
			scrolls[which].know = true;
			message(L"your nose tingles and you smell food.", Color::White);
		}

		else
			message(L"your nose tingles.", Color::White);
	}
		break;

	case Scroll::Teleport:
	{
		// TODO: 랜덤 위치이 아닌 랜덤 룸의 랜덤 위치로 변경
		sf::Vector2i oldPos = actor.getPosition();
		
		teleport(actor);
		
		if (actor.getPosition() != oldPos)
			scrolls[which].know = true;
	}
		break;

	case Scroll::EnchantWeapon:
		if (Item* weapon = actor.getEquipment(Actor::Weapon)) // && weapon->getType() == ItemType::Weapon
		{
			weapon->enchant();
			weapon->removeFlag(Item::IsCursed);

			message(L"your " + weapon->getName() + L" glows " + Potion::pickColor(actor, L"blue") + L" for a moment.", Color::White);
		}

		else
			message(L"you feel a strange sense of loss.", Color::White);
		break;

	case Scroll::Create:
	{
		std::vector<sf::Vector2i> points;

		for (int dy = -1; dy <= 1; ++dy)
			for (int dx = -1; dx <= 1; ++dx)
			{
				if (dx == 0 && dy == 0)
					continue;

				sf::Vector2i pos = actor.getPosition() + sf::Vector2i(dx, dy);

				if (level.at(pos).passable && !level.actorAt(pos))
				{
					Item* item = level.itemAt(pos);

					if (item && item->isSame(ItemType::Scroll, Scroll::Scare))
						continue;

					points.emplace_back(pos);
				}
			}

		if (!points.empty())
		{
			scrolls[which].know = true;

			auto monster = Actor::createMonster(getWorld().getDepth(), false);
			sf::Vector2i pos = points[randomInt(points.size())];
			monster->setPosition(pos);
			level.attach(std::move(monster));
		}

		else
			message(L"you hear a faint cry of anguish in the distance.", Color::White);
	}
		break;

	case Scroll::Remove:
	{
		bool removed = false;

		for (int i = 0; i < Actor::MaxSlot; ++i)
		{
			auto slot = static_cast<Actor::Slot>(i);
			Item* item = actor.getEquipment(slot);
			
			if (item && item->hasFlag(Item::IsCursed))
			{
				item->removeFlag(Item::IsCursed);
				removed = true;
			}
		}

		if (removed)
			scrolls[which].know = true;

		message(actor.chooseStr(L"you feel in touch with the Universal Onenes.",
								L"you feel as if somebody is watching over you."), Color::White);
	}
		break;

	case Scroll::Aggravate:
		// TODO: 몬스터가 없는 경우?
		getWorld().aggravate();
		message(L"you hear a high pitched humming noise.", Color::White);
		break;

	case Scroll::Protect:
		if (Item* armor = actor.getEquipment(Actor::Armor))
		{
			scrolls[which].know = true;
			armor->addFlag(Item::IsProtected);
			message(L"your armor is covered by a shimmering " + Potion::pickColor(actor, L"gold") + L" shield.", Color::White);
		}

		else
			message(L"you feel a strange sense of loss.", Color::White);
		break;

	default:
		message(L"what a puzzling scroll!", Color::White);
		break;
	}

	return true;
}

void Scroll::identify()
{
	scrolls[which].know = true;
}

bool Scroll::isIdentified() const
{
	return scrolls[which].know;
}

void Scroll::initNames(Rng& rng)
{
	/* init.c */
	// init_names: Generate the names of the various scrolls

	names.resize(scrolls.size());

	for (auto& name : names)
	{
		name.clear();

		int nwords = rng.getInt(3) + 2;

		while (nwords--)
		{
			int nsyl = rng.getInt(3) + 1;

			while (nsyl--)
			{
				const auto& sp = rng.getOne(sylls);

				if (name.size() + sp.size() > MaxNames)
					break;

				name += sp;
			}

			name += L' ';
		}

		// HACK:
		while (name.back() == L' ')
			name.pop_back();
	}
}

int Scroll::pickOne(Rng& rng)
{
	return Item::pickOne(rng, scrolls);
}
