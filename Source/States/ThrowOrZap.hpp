#pragma once

#include "State.hpp"
#include "../Utility/Direction.hpp"

#include <functional>

class Item;

class ThrowOrZap : public State
{
public:
	using Callback = std::function<std::unique_ptr<Item>(Direction)>;

public:
	explicit ThrowOrZap(Callback callback);
	ThrowOrZap(std::unique_ptr<Item> object, Direction dir);

	void handleKeyboard(sf::Keyboard::Key key) override;
	void update(sf::Time dt) override;
	void drawBefore() override;

private:
	Callback callback;
	std::unique_ptr<Item> object = nullptr;
	Direction dir;
	sf::Time elapsedTime;
};
