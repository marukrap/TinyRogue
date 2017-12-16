#include "ThrowOrZap.hpp"
#include "StateStack.hpp"
#include "../World/World.hpp"
#include "../Utility/Utility.hpp"

namespace
{
	const sf::Time secondsPerTick = sf::seconds(0.05f);
}

ThrowOrZap::ThrowOrZap(Callback callback)
	: callback(std::move(callback))
{
}

ThrowOrZap::ThrowOrZap(std::unique_ptr<Item> object, Direction dir)
	: object(std::move(object))
	, dir(dir)
	, elapsedTime(secondsPerTick)
{
}

void ThrowOrZap::handleKeyboard(sf::Keyboard::Key key)
{
	if (object)
		return;

	// TODO: Player class에서 키 바인딩을 참조하여 사용할 것

	dir = Direction::None;

	switch (key)
	{
	case sf::Keyboard::Escape:
	case sf::Keyboard::Insert:
	case sf::Keyboard::Numpad0:
		getLog().popBack();
		stack->popState();
		break;

	case sf::Keyboard::Left:
	case sf::Keyboard::Numpad4:
	case sf::Keyboard::H:
		dir = Direction::W;
		break;

	case sf::Keyboard::Right:
	case sf::Keyboard::Numpad6:
	case sf::Keyboard::L:
		dir = Direction::E;
		break;

	case sf::Keyboard::Up:
	case sf::Keyboard::Numpad8:
	case sf::Keyboard::K:
		dir = Direction::N;
		break;

	case sf::Keyboard::Down:
	case sf::Keyboard::Numpad2:
	case sf::Keyboard::J:
		dir = Direction::S;
		break;

	case sf::Keyboard::Home:
	case sf::Keyboard::Numpad7:
	case sf::Keyboard::Y:
		dir = Direction::NW;
		break;

	case sf::Keyboard::End:
	case sf::Keyboard::Numpad1:
	case sf::Keyboard::B:
		dir = Direction::SW;
		break;

	case sf::Keyboard::PageUp:
	case sf::Keyboard::Numpad9:
	case sf::Keyboard::U:
		dir = Direction::NE;
		break;

	case sf::Keyboard::PageDown:
	case sf::Keyboard::Numpad3:
	case sf::Keyboard::N:
		dir = Direction::SE;
		break;
	}

	if (dir != Direction::None)
	{
		getLog().popBack();

		object = std::move(callback(dir));

		if (!object)
		{
			stack->popState();
			getWorld().endPlayerTurn();
		}

		else
			elapsedTime = secondsPerTick;
	}
}

#include "../World/ActorPlayer.hpp"
#include "../World/Items/Weapon.hpp"
#include "../Action/AttackAction.hpp"

void ThrowOrZap::update(sf::Time dt)
{
	if (!object)
		return;

	elapsedTime += dt;

	if (elapsedTime < secondsPerTick)
		return;

	elapsedTime -= secondsPerTick;

	object->move(dir);

	Level& level = getLevel();
	sf::Vector2i pos = object->getPosition();

	// if (!level.at(pos).passable || level.at(pos).type == Tile::Door)
	if (level.at(pos).type != Tile::Floor)
	{
		if (object->isSame(ItemType::Weapon, Weapon::Flame))
			message(L"{0} vanishes with a puff of smoke.", { *object });
		
		else
		{
			// UNDONE: fall: fallpos:

			std::vector<Direction> dirs;

			dir = -dir;
			auto left = dir.left45();
			auto right = dir.right45();

			// TODO: 같은 group의 아이템은 겹침 (epyx rogue)
			
			if (!level.actorAt(pos + dir) && !level.itemAt(pos + dir))
				dirs.emplace_back(dir);

			if (level.at(pos + left).type == Tile::Floor && !level.itemAt(pos + left))
				dirs.emplace_back(left);

			if (level.at(pos + right).type == Tile::Floor && !level.itemAt(pos + right))
				dirs.emplace_back(right);

			if (!dirs.empty())
			{
				pos += dirs[randomInt(dirs.size())];
				object->setPosition(pos);
				level.attach(std::move(object));
			}

			else
			{
				Word theName(L"the " + object->getName(), object->getColor());
				message(L"{0} vanishes as it hits the ground.", { theName });
			}
		}

		stack->popState();
		getWorld().endPlayerTurn();
	}

	// TODO: 적이 문 위에 있는 경우?

	else
	{
		Actor* actor = level.actorAt(pos);

		if (actor)
		{
			if (object->isSame(ItemType::Weapon, Weapon::Flame))
			{
				if (!actor->throwCheck(Actor::Magic))
				{
					// REMOVE:
					if (actor->getType() != Actor::Hero)
						actor->startRun();

					actor->takeDamage(rollDice(6, 6));

					// TODO: AttackAction의 함수를 사용해서 공격, 경험치 획득 등
					// hit_monster() > fight()
					message(L"{0} hits {1}.", { *object, *actor });

					if (actor->isDead())
						message(L"{0} killed {1}.", { *object, *actor });
				}

				else
				{
					if (actor->getType() != Actor::Hero)
						actor->startRun();

					message(L"{0} whizzes past {1}.", { *object, *actor });
				}
			}

			else
			{
				// TODO: 혼란 상태에서 투척, 지팡이 휘두르기 등의 처리를 어떻게 할 것인지

				std::make_unique<AttackAction>(*actor, dir, object.get())->perform(getWorld().getPlayerActor());

				if (object->getCount() >= 1)
				{
					// UNDONE: fall: fallpos:

					std::vector<Direction> dirs;

					for (int dy = -1; dy <= 1; ++dy)
						for (int dx = -1; dx <= 1; ++dx)
						{
							Direction dir(dx, dy);

							if (level.at(pos + dir).type == Tile::Floor && !level.itemAt(pos + dir))
								dirs.emplace_back(dir);
						}

					if (!dirs.empty())
					{
						pos += dirs[randomInt(dirs.size())];
						object->setPosition(pos);
						level.attach(std::move(object));
					}

					else
					{
						Word theName(L"the " + object->getName(), object->getColor());
						message(L"{0} vanishes as it hits the ground.", { theName });
					}
				}
			}

			// TODO: 관통
			stack->popState();
			getWorld().endPlayerTurn();
		}
	}
}

void ThrowOrZap::drawBefore()
{
	if (object && getWorld().isVisible(*object))
		object->draw(*console);
}
