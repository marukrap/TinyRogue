#pragma once

#include "../Game/GameObject.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Time.hpp>

#include <string>

class Console;

class Entity : public GameObject
{
public:
	explicit Entity(wchar_t ch);
	virtual ~Entity() = default;

	// NonCopyable
	Entity(const Entity&) = delete;
	Entity& operator=(const Entity&) = delete;

	virtual std::wstring getName() const;
	virtual std::wstring getAName() const; // used for items
	virtual std::wstring getTheName() const; // used for actors

	void setChar(wchar_t ch);
	wchar_t getChar() const;

	void setPosition(int x, int y);
	void setPosition(const sf::Vector2i& pos);
	const sf::Vector2i& getPosition() const;

	void move(int dx, int dy);
	void move(const sf::Vector2i& dir);

	void setColor(const sf::Color& color);
	virtual const sf::Color& getColor() const;

	void update(sf::Time dt);
	virtual void draw(Console& console);

private:
	wchar_t ch;
	sf::Vector2i position;
	sf::Color color = sf::Color::White;
};
