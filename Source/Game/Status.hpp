#pragma once

// [State]			// hungry_state(hungry, weak, faint) or title
// Depth: 1			// level
// HP: 12/12		// hpt / hpmax
// Str: 16 Arm: 4	// str / 10 - arm
// Lvl: 1 Exp: 0	// lvl / exp

// Gold:			// purse

#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include <array>

class Actor;

class Status : public sf::Drawable, public sf::Transformable
{
public:
	enum Type
	{
		State,
		HitPoints,
		Strength,
		Experience,
		Depth,
		TypeCount
	};

public:
	Status(int width, int height, const sf::Font& font, int fontSize, const sf::Vector2f& tileSize);

	void setState(const std::wstring& str, const sf::Color& color);
	void setStats(const Actor& actor);
	void setDepth(int depth);

private:
	void setHp(int hp, int maxHp);
	void setStr(int str, int maxStr, int arm);
	void setExp(int lvl, int exp);

private:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	void centerText(sf::Text& text);

private:
	sf::Vector2i size;
	sf::Vector2f tileSize;

	sf::RectangleShape hpBar;
	std::array<sf::Text, TypeCount> texts;
};
