#include "Help.hpp"
#include "StateStack.hpp"
#include "../Game/Console.hpp"
#include "../Graphics/Color.hpp"

namespace
{
	using Pair = std::pair<std::wstring, std::wstring>;

	const std::vector<std::vector<Pair>> pages =
	{
		{
			{ L"         ", L"" },
			{ L"         ", L"- Standard -" },
			{ L"         ", L"" },
			{ L"    F1   ", L"display help" },
			{ L"         ", L"" },
			{ L"  7 8 9  ", L"" },
			{ L"  4   6  ", L"move or attack" },
			{ L"  1 2 3  ", L"" },
			{ L"         ", L"" },
			{ L"  Space  ", L"interact with current tile (pick up items, use stairs or search)" },
			{ L"    z    ", L"rest for a turn (Z to rest for 100 turns)" },
			{ L"    i    ", L"show inventory" },
			{ L"    f    ", L"fire thrown item again (if available)" },
			{ L"         ", L"" },
			{ L"         ", L"- Inventory -" },
			{ L"         ", L"" },
			{ L"  Space  ", L"choose an action" },
			// { L"       a ", L"apply selected item" },
			// { L"       d ", L"drop selected item" },
			// { L"       t ", L"throw selected item" },
		},

		{
			{ L"         ", L"" },
			{ L"         ", L"- Classic -" },
			{ L"         ", L"" },
			{ L"    ?    ", L"display help" },
			{ L"         ", L"" },
			{ L"  y k u  ", L"" },
			{ L"  h   l  ", L"move or attack" },
			{ L"  b j n  ", L"" },
			{ L"         ", L"" },
			{ L"    ,    ", L"pick something up (also g)" },
			{ L"    .    ", L"rest for a turn" },
			{ L"    s    ", L"search for trap/secret door" },
			{ L"    a    ", L"use an item (quaff, read, eat, equip, remove or zap)" },
			{ L"    d    ", L"drop object" },
			{ L"    t    ", L"throw something" },
			{ L"    >    ", L"go down a staircase" },
			{ L"    <    ", L"go up a staircase" },
		},

		{
			{ L"         ", L"" },
			{ L"         ", L"- Numpad -" },
			{ L"         ", L"" },
			{ L"  7 8 9  ", L"" },
			{ L"  4   6  ", L"move or attack" },
			{ L"  1 2 3  ", L"" },
			{ L"         ", L"" },
			{ L" Numpad0 ", L"show inventory or cancel command" },
			{ L" Numpad5 ", L"rest for a turn" },
			{ L"  Enter  ", L"interact with current tile" },
		},
	};
}

void Help::handleKeyboard(sf::Keyboard::Key key)
{
	switch (key)
	{
	case sf::Keyboard::Escape:
	case sf::Keyboard::Insert:
	case sf::Keyboard::Numpad0:
	case sf::Keyboard::F1:
		stack->popState();
		break;

	case sf::Keyboard::Slash:
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift))
			stack->popState();
		break;

	case sf::Keyboard::Left:
	case sf::Keyboard::Up:
		if (page > 0)
			--page;
		break;

	case sf::Keyboard::Right:
	case sf::Keyboard::Down:
		if (page < pages.size() - 1)
			++page;
		break;
	}
}

void Help::update(sf::Time dt)
{
}

void Help::drawAfter()
{
	int width = console->getSize().x - 2;
	int height = console->getSize().y - 2;
	int left = 1;
	int top = 1;

	drawRect(left, top, width, height);

	std::wstring header = L" Commands (" + std::to_wstring(page + 1) + L"/3) ";
	std::wstring footer = L" Press ";

	if (page == 0)
		footer += L"Right";
	else if (page == pages.size() - 1)
		footer += L"Left";
	else
		footer += L"Left/Right";

	footer += L" to more, Esc to exit ";

	console->setString(left + width / 2 - header.size() / 2, top, header);
	console->setString(left + width / 2 - footer.size() / 2, top + height - 1, footer, Color::DarkGray);
	console->setColor(left + width / 2 - header.size() / 2 + 11, top, Color::Yellow, Console::TextLayer);

	// TODO: D to list discovered items

	for (const auto& pair : pages[page])
	{
		console->setString(left + 2, ++top, pair.first, Color::Yellow);
		console->setString(left + 2 + pair.first.size() + 1, top, pair.second);
	}

	// HACK: Colorize 'Z' and 'g'
	if (page == 0)
		console->setColor(30, 12, Color::Yellow, Console::TextLayer);
	else if (page == 1)
		console->setColor(37, 11, Color::Yellow, Console::TextLayer);
}
