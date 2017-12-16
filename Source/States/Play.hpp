#pragma once

#include "State.hpp"
#include "../World/World.hpp"

class Play : public State
{
public:
	Play();

	void handleKeyboard(sf::Keyboard::Key key) override;
	void update(sf::Time dt) override;
	void drawBefore() override;

private:
	World world;
};
