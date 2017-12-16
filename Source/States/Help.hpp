#pragma once

#include "State.hpp"

class Help : public State
{
public:
	void handleKeyboard(sf::Keyboard::Key key) override;
	void update(sf::Time dt) override;
	void drawAfter() override;

private:
	std::size_t page = 0;
};
