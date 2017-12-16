#pragma once

#include "State.hpp"

class Title : public State
{
public:
	Title();

	void handleKeyboard(sf::Keyboard::Key key) override;
	void update(sf::Time dt) override;
};
