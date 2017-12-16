#pragma once

#include "../Game/GameObject.hpp"

#include <SFML/Window/Keyboard.hpp>
#include <SFML/System/Time.hpp>

#include <memory>

class StateStack;
class Console;

class State : public GameObject
{
public:
	using Ptr = std::unique_ptr<State>;

public:
	virtual ~State() = default;

	virtual void handleKeyboard(sf::Keyboard::Key key) = 0;
	virtual void update(sf::Time dt) = 0;
	virtual void drawBefore();
	virtual void drawAfter();

	static void initialize(StateStack& stack, Console& console);

protected:
	void drawRect(int left, int top, int width, int height);

protected:
	static StateStack* stack;
	static Console* console;
};
