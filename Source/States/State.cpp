#include "State.hpp"
#include "../Game/Console.hpp"

StateStack* State::stack = nullptr;
Console* State::console = nullptr;

void State::drawBefore()
{
}

void State::drawAfter()
{
}

void State::initialize(StateStack& stack, Console& console)
{
	State::stack = &stack;
	State::console = &console;
}

void State::drawRect(int left, int top, int width, int height)
{
	const sf::Color bgColor(0, 0, 0, 200);

	int right = left + width;
	int bottom = top + height;

	for (int y = top; y < bottom; ++y)
		for (int x = left; x < right; ++x)
		{
			console->setChar(x, y, L' ');
			console->setColor(x, y, bgColor, Console::Background);
		}

	for (int x = left + 1; x < right - 1; ++x)
	{
		console->setChar(x, top, L'-');
		console->setChar(x, bottom - 1, L'-');
	}

	for (int y = top + 1; y < bottom - 1; ++y)
	{
		console->setChar(left, y, L'|');
		console->setChar(right - 1, y, L'|');
	}

	console->setChar(left, top, L'+');
	console->setChar(right - 1, top, L'+');
	console->setChar(left, bottom - 1, L'+');
	console->setChar(right - 1, bottom - 1, L'+');
}
