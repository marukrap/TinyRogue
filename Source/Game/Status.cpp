#include "Status.hpp"
#include "../World/Actor.hpp"
#include "../Graphics/Color.hpp"

#include <SFML/Graphics/RenderTarget.hpp>

#include <iostream>
#include <cmath> // ceil

Status::Status(int width, int height, const sf::Font& font, int fontSize, const sf::Vector2f& tileSize)
	: size(width, height)
	, tileSize(tileSize)
{
	sf::FloatRect bounds = sf::Text(L'A', font, fontSize).getLocalBounds();
	float topSpace = bounds.top;
	float bottomSpace = tileSize.y - (bounds.top + bounds.height);
	sf::Vector2f origin(0.f, std::floor((topSpace - bottomSpace) / 2.f));
	std::cout << "Status Origin: " << origin.x << ", " << origin.y << '\n';

	hpBar.setFillColor(Color::Blood);
	hpBar.setPosition(0.f, tileSize.y * HitPoints);

	for (std::size_t i = 0; i < texts.size(); ++i)
	{
		texts[i].setFont(font);
		texts[i].setCharacterSize(fontSize);
		texts[i].setPosition(0.f, tileSize.y * i);
		texts[i].setOrigin(origin);
	}
}

void Status::setState(const std::wstring& str, const sf::Color& color)
{
	texts[State].setString(str);
	texts[State].setFillColor(color);
	centerText(texts[State]);
}

void Status::setStats(const Actor& actor)
{
	setHp(actor.getHp(), actor.getMaxHp());
	setStr(actor.getStr(), actor.getMaxStr(), actor.getArmor());
	setExp(actor.getLevel(), actor.getExp());
}

void Status::setDepth(int depth)
{
	texts[Depth].setString(L"Depth " + std::to_wstring(depth));
	centerText(texts[Depth]);
}

void Status::setHp(int hp, int maxHp)
{
	float width = std::ceil(static_cast<float>(hp) / maxHp * size.x);
	hpBar.setSize(sf::Vector2f(width * tileSize.x, tileSize.y));

	if (hp <= 0)
	{
		texts[HitPoints].setString(L"Dead");
		texts[HitPoints].setFillColor(Color::Blood);
	}

	else
	{
		texts[HitPoints].setString(L"HP " + std::to_wstring(hp) + L"/" + std::to_wstring(maxHp));
		texts[HitPoints].setFillColor(sf::Color::White);
	}

	centerText(texts[HitPoints]);
}

void Status::setStr(int str, int maxStr, int arm)
{
	if (str == maxStr)
		texts[Strength].setString(L"Str " + std::to_wstring(str) + L"  Arm " + std::to_wstring(10 - arm));
	else
		texts[Strength].setString(L"Str " + std::to_wstring(str) + L"/" + std::to_wstring(maxStr) + L"  Arm " + std::to_wstring(10 - arm));

	centerText(texts[Strength]);
}

void Status::setExp(int lvl, int exp)
{
	texts[Experience].setString(L"Lvl " + std::to_wstring(lvl) + L"  Exp " + std::to_wstring(exp));

	centerText(texts[Experience]);
}

void Status::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();

	target.draw(hpBar, states);

	for (const auto& text : texts)
		target.draw(text, states);
}

void Status::centerText(sf::Text& text)
{
	const auto& str = text.getString();
	const auto& pos = text.getPosition();

	int x = size.x / 2 - str.getSize() / 2;
	
	text.setPosition(x * tileSize.x, pos.y);
}
