#pragma once

#include <SFML/Graphics/Color.hpp>

struct Tile
{
	enum Type : wchar_t
	{
		Unused		= L' ',
		Floor		= 0x00b7, // L'.',
		Corridor	= L',',
		Wall		= L'#',
		Door		= L'+',
		UpStairs	= 0x00ab, // L'<',
		DownStairs	= 0x00bb, // L'>',
		// Trap		= L'^',
	};

	Type type = Unused;
	sf::Color color = sf::Color::White;
	bool passable = false;
	bool transparent = false;
	bool visible = false;
	bool explored = false;
};
