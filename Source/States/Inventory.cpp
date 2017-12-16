#include "Inventory.hpp"
#include "StateStack.hpp"
#include "../Game/Console.hpp"
#include "../World/World.hpp"
#include "../Action/ItemAction.hpp"
#include "../Graphics/Color.hpp"

#include <cwctype> // iswalpha, towupper

namespace
{
	const std::wstring hotkeys = L"abcdefghijklmnopqrstuvwxyz";
}

Inventory::Inventory(Actor& actor, Mode mode)
	: actor(actor)
	, mode(mode)
{
	switch (mode)
	{
	case Normal:
	case Apply:
	case Drop:
		for (const auto& item : actor.pack)
			items.emplace_back(item.get());
		break;

	case Throw:
		for (const auto& item : actor.pack)
			if (item->getType() == ItemType::Weapon)
				items.emplace_back(item.get());
		break;

	case IdPotion:
		for (const auto& item : actor.pack)
			if (item->getType() == ItemType::Potion && !item->isIdentified())
				items.emplace_back(item.get());
		break;

	case IdScroll:
		for (const auto& item : actor.pack)
			if (item->getType() == ItemType::Scroll && !item->isIdentified())
				items.emplace_back(item.get());
		break;

	case IdWeapon:
		for (const auto& item : actor.pack)
			if (item->getType() == ItemType::Weapon && !item->isIdentified())
				items.emplace_back(item.get());
		break;

	case IdArmor:
		for (const auto& item : actor.pack)
			if (item->getType() == ItemType::Armor && !item->isIdentified())
				items.emplace_back(item.get());
		break;

	case IdRingOrStick:
		for (const auto& item : actor.pack)
			if ((item->getType() == ItemType::Ring || item->getType() == ItemType::Stick) && !item->isIdentified())
				items.emplace_back(item.get());
		break;
	}
}

void Inventory::handleKeyboard(sf::Keyboard::Key key)
{
	if (currentCommand >= 0)
	{
		switch (key)
		{
		case sf::Keyboard::Escape:
		case sf::Keyboard::Insert:
		case sf::Keyboard::Numpad0:
			currentCommand = -1; // Close popup
			break;

		case sf::Keyboard::Up:
		case sf::Keyboard::Left:
		case sf::Keyboard::Numpad8:
		case sf::Keyboard::Numpad4:
			currentCommand = (currentCommand + commands.size() - 1) % commands.size();
			playSound(SoundID::Bell);
			break;

		case sf::Keyboard::Down:
		case sf::Keyboard::Right:
		case sf::Keyboard::Numpad2:
		case sf::Keyboard::Numpad6:
			currentCommand = (currentCommand + 1) % commands.size();
			playSound(SoundID::Bell);
			break;

		case sf::Keyboard::Space:
		case sf::Keyboard::Return:
			stack->popState();

			if (currentCommand == 1) // Drop
				drop();
			else if (currentCommand == 2) // Throw
				hurl();
			else // currentCommand == 0 // Apply
				use();

			playSound(SoundID::Bell);
			break;
		}

		return;
	}

	switch (key)
	{
	case sf::Keyboard::Escape:
	// case sf::Keyboard::I:
	case sf::Keyboard::Insert:
	case sf::Keyboard::Numpad0:
		if (selected >= 0 && mode >= IdPotion)
			message(L"you must identify something.", Color::White);
		else
			stack->popState();
		break;

	case sf::Keyboard::Up:
	case sf::Keyboard::Left:
	case sf::Keyboard::Numpad8:
	case sf::Keyboard::Numpad4:
		if (selected >= 0)
		{
			selected = (selected + items.size() - 1) % items.size();

			if (items.size() > 1)
				playSound(SoundID::Bell);
		}
		break;

	case sf::Keyboard::Down:
	case sf::Keyboard::Right:
	case sf::Keyboard::Numpad2:
	case sf::Keyboard::Numpad6:
		if (selected >= 0)
		{
			selected = (selected + 1) % items.size();

			if (items.size() > 1)
				playSound(SoundID::Bell);
		}
		break;

	case sf::Keyboard::Space:
	case sf::Keyboard::Return:
		if (selected >= 0 && !actor.isDead())
		{
			if (mode == Normal)
			{
				commands.clear();
				currentCommand = 0;

				Item& item = *items[selected];
				std::wstring command;

				switch (item.getType())
				{
				case ItemType::Potion:
					command = L"Drink";
					break;

				case ItemType::Scroll:
					command = L"Read";
					break;

				case ItemType::Food:
					command = L"Eat";
					break;

				case ItemType::Weapon:
				case ItemType::Armor:
				case ItemType::Ring:
					command = L"Equip";

					for (int j = 0; j < Actor::MaxSlot; ++j)
					{
						if (actor.getEquipment(static_cast<Actor::Slot>(j)) == &item)
						{
							command = L"Remove";
							break;
						}
					}
					break;

				case ItemType::Stick:
					command = L"Zap";
					break;

				default:
					command = L"Use";
					break;
				}

				commands.emplace_back(command);
				commands.emplace_back(L"Drop");
				commands.emplace_back(L"Throw");

				playSound(SoundID::Bell);
			}

			else
			{
				stack->popState();

				if (mode == Drop)
					drop();
				else if (mode == Throw)
					hurl();
				else
					use();

				playSound(SoundID::Bell);
			}
		}
		break;
	}

	if (!isClassic())
		return;

	if (selected >= 0 && !actor.isDead() && key >= sf::Keyboard::A && key <= sf::Keyboard::Z)
	{
		wchar_t ch = L'a' + (key - sf::Keyboard::A);

		for (std::size_t i = 0; i < hotkeys.size(); ++i)
		{
			if (hotkeys[i] == ch && i < items.size())
			{
				stack->popState();
				selected = i;

				if (mode == Drop)
					drop();
				else if (mode == Throw)
					hurl();
				else
					use();
				
				playSound(SoundID::Bell);
				break;
			}
		}
	}
}

void Inventory::update(sf::Time dt)
{
}

void Inventory::drawAfter()
{
	int width = 40; // console->getSize().x / 2;
	int height = items.size() + 2;
	int left = 21;
	int top = items.size() >= 23 ? 0 : 1;

	if (height <= 2)
		height = 3;
	else if (selected < 0)
		selected = 0;

	drawRect(left, top, width, height);

	std::wstring header;

	if (mode == Apply)
		header += L" Use what? ";
	else if (mode == Drop)
		header += L" Drop what? ";
	else if (mode == Throw)
		header += L" Throw what? ";
	else
		header += L" Inventory ";
	
	std::size_t goldSign = header.size();

	header += L"$ " + std::to_wstring(actor.purse) + L" ";

	std::wstring footer = L" (" + std::to_wstring(actor.getNumItems()) + L"/" + std::to_wstring(Actor::MaxPackSize) + L") ";

	console->setString(left + (width - header.size()) / 2, top, header);
	console->setString(left + (width - footer.size()) / 2, top + height - 1, footer, Color::DarkGray);
	console->setColor(left + (width - header.size()) / 2 + goldSign, top, Color::Yellow, Console::TextLayer);

	for (std::size_t i = 0; i < items.size(); ++i)
	{
		Item& item = *items[i];
		std::wstring name;

		// "a) "
		if (isClassic())
		{
			if (i < Actor::MaxPackSize)
				name += hotkeys[i];
			else
				name += L'?';

			name += L") ";
		}

		// "> "
		else
		{
			if (i == selected)
				name += L"> ";
			else
				name += L"  ";
		}

		std::size_t charSign = name.size();

		name += item.getChar();
		name += L' ';

		std::size_t nameBegin = name.size();

		name += item.getAName();

		if (std::iswalpha(name[nameBegin]))
			name[nameBegin] = std::towupper(name[nameBegin]);

		if (static_cast<int>(name.size()) > width - 4)
		{
			name.resize(width - 7);
			name += L"...";
		}

		// TODO: 장착하고 있는 저주 아이템은 식별되지 않은 경우에도 붉은 색으로 표시

		int x = left + 2;
		int y = top + 1 + i;

		if (item.isIdentified())
		{
			if (item.hasFlag(Item::IsCursed))
				console->setString(x, y, name, Color::Red);
			else
				console->setString(x, y, name);
		}

		else
			console->setString(x, y, name, Color::White);

		console->setColor(x, y, 2, 1, sf::Color::White, Console::TextLayer);
		console->setColor(x + charSign, y, item.getColor(), Console::TextLayer);

		for (int j = 0; j < Actor::MaxSlot; ++j)
		{
			if (actor.getEquipment(static_cast<Actor::Slot>(j)) == &item)
			{
				std::wstring str = (j == Actor::Weapon ? L"(in hand)" : L"(worn)");
				console->setString(x + name.size() + 1, y, str, Color::DarkGray);
				break;
			}
		}

		if (i == selected)
			console->setColor(x + nameBegin, y, name.size() - nameBegin, 1, Color::DarkViolet);
	}

	if (items.empty())
	{
		std::wstring str = (mode == Normal ? L"Your pack is empty" : L"You don't have anything appropriate");
		console->setString(left + (width - str.size()) / 2, top + 1, str, Color::DarkGray);
	}

	if (currentCommand >= 0)
	{
		int x = left + width;
		int y = 1;

		drawRect(x, y, 12, 5);

		for (std::size_t i = 0; i < commands.size(); ++i)
		{
			if (i == currentCommand)
				console->setString(x + 2, y + 1 + i, L"> " + commands[i]);
			else
				console->setString(x + 2, y + 1 + i, L"  " + commands[i]);
		}

		console->setColor(x + 4, y + 1 + currentCommand, commands[currentCommand].size(), 1, Color::DarkViolet);
	}
}

bool Inventory::isClassic() const
{
	return mode == Apply || mode == Drop || mode == Throw;
}

void Inventory::use()
{
	Item& item = *items[selected];

	if (mode == Drop)
	{
		drop();
		return;
	}

	else if (mode == Throw)
	{
		hurl();
		return;
	}

	else if (mode >= IdPotion)
	{
		item.identify();
		message(L"this is {0}.", { item });
		getWorld().endPlayerTurn();
	}

	else // Use
	{
		if (item.getType() == ItemType::Stick)
		{
			std::make_unique<UseAction>(item)->perform(actor);
			return; // Do no remove this code, Inventory will be destructed after 'perform'
		}

		if (std::make_unique<UseAction>(item)->perform(actor))
			getWorld().endPlayerTurn();
	}
}

void Inventory::drop()
{
	Item& item = *items[selected];

	if (item.dropCheck(actor))
		std::make_unique<DropAction>(item)->perform(actor);

	getWorld().endPlayerTurn();
}

void Inventory::hurl()
{
	Item& item = *items[selected];

	if (item.dropCheck(actor))
		std::make_unique<ThrowAction>(item)->perform(actor);
}
