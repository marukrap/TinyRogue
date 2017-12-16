#include "ItemAction.hpp"
#include "../World/World.hpp"
#include "../World/Items/Scroll.hpp"
#include "../Graphics/Color.hpp"

PickUpAction::PickUpAction()
{
}

PickUpAction::PickUpAction(Item& item)
	: item(&item)
{
}

bool PickUpAction::perform(Actor& actor)
{
	if (!item)
		item = getLevel().itemAt(actor.getPosition());

	if (!item)
	{
		message(L"there is nothing here to pick up.", Color::White);
		return false;
	}

	if (actor.levitCheck())
		return false;

	if (item->getType() == ItemType::Gold)
	{
		/* pack.c */
		// money: Add or subtract gold from the pack

		message(L"{0} found {1}.", { actor, *item });
		playSound(SoundID::Gold);

		auto itemPtr = getLevel().detach(*item);
		actor.addPack(std::move(itemPtr));
	}

	else
	{
		// add_pack: Pick up an object and add it to the pack

		// Add the item count if there's the same group in the pack
		if (actor.getNumItems() >= Actor::MaxPackSize && !actor.checkGroup(*item))
		{
			// message(L"there's no room in your pack.", Color::White);
			message(L"your pack is too full to pick up {0}.", { *item });
			return false;
		}

		auto itemPtr = getLevel().detach(*item);

		if (item->isSame(ItemType::Scroll, Scroll::Scare) && item->hasFlag(Item::IsFound))
		{
			message(L"the scroll turns to dust as you pick it up.", Color::White);
			return true;
		}

		item = actor.addPack(std::move(itemPtr));
		item->addFlag(Item::IsFound);
		item->removeFlag(Item::IsDetected);

		message(L"{0} now have {1}.", { actor, *item });
	}

	return true;
}

UseAction::UseAction(Item& item)
	: item(item)
{
}

bool UseAction::perform(Actor& actor)
{
	bool result = item.use(actor);

	if (item.getCount() <= 0)
		actor.unpack(item);

	getWorld().updateStatus();

	return result;
}

DropAction::DropAction(Item& item)
	: item(item)
{
}

bool DropAction::perform(Actor& actor)
{
	if (getLevel().at(actor.getPosition()).type != Tile::Floor)
	{
		message(L"there is something there already.", Color::White);
		return false;
	}

	Item::Ptr itemPtr = actor.unpack(item);

	message(L"{0} dropped {1}.", { actor, item });

	Item* itemOnFloor = getLevel().itemAt(actor.getPosition());

	if (itemOnFloor)
		std::make_unique<PickUpAction>(*itemOnFloor)->perform(actor);

	itemPtr->setPosition(actor.getPosition());
	getLevel().attach(std::move(itemPtr));

	return true;
}

#include "../States/ThrowOrZap.hpp"

ThrowAction::ThrowAction(Item& item)
	: item(item)
{
}

bool ThrowAction::perform(Actor& actor)
{
	/* weapons.c */
	// missile: Fire a missile in a given direction

	// TODO: i로 인벤토리를 열어 t로 던지는 경우 모든 아이템을 던질 수 있게 되는 문제
	//       (현재 프로그램에서는 무기만 허용)

	message(L"which direction?", Color::Blue);

	// https://stackoverflow.com/questions/7895879/using-member-variable-in-lambda-capture-list-inside-a-member-function
	ThrowOrZap::Callback callback = [&actor, &item = item] (Direction dir)
	{
		auto missile = actor.unpack(item, true);
		missile->removeFlag(Item::IsFound); // HACK: Auto pick up
		missile->setPosition(actor.getPosition());

		return missile;
	};

	pushState(std::make_unique<ThrowOrZap>(std::move(callback)));

	return true;
}
