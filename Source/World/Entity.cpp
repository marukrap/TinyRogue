#include "Entity.hpp"
#include "../Game/Console.hpp"

Entity::Entity(wchar_t ch)
	: ch(ch)
{
}

std::wstring Entity::getName() const
{
	return L"Entity::getName()";
}

std::wstring Entity::getAName() const
{
	return L"Entity::getAName()";
}

std::wstring Entity::getTheName() const
{
	return L"Entity::getTheName()";
}

void Entity::setChar(wchar_t ch)
{
	this->ch = ch;
}

wchar_t Entity::getChar() const
{
	return ch;
}

void Entity::setPosition(int x, int y)
{
	position.x = x;
	position.y = y;
}

void Entity::setPosition(const sf::Vector2i& pos)
{
	setPosition(pos.x, pos.y);
}

const sf::Vector2i & Entity::getPosition() const
{
	return position;
}

void Entity::move(int dx, int dy)
{
	position.x += dx;
	position.y += dy;
}

void Entity::move(const sf::Vector2i& dir)
{
	move(dir.x, dir.y);
}

void Entity::setColor(const sf::Color& color)
{
	this->color = color;
}

const sf::Color& Entity::getColor() const
{
	return color;
}

void Entity::update(sf::Time dt)
{
	// Do nothing by default
}

void Entity::draw(Console & console)
{
	console.setChar(position.x, position.y, ch, color);
}
