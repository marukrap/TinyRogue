#pragma once

#include "../Action/Action.hpp"

#include <SFML/Window/Keyboard.hpp>

#include <unordered_map>
#include <functional>

enum class Command
{
	Unknown,
	MoveLeft,
	MoveRight,
	MoveUp,
	MoveDown,
	MoveLeftUp,
	MoveLeftDown,
	MoveRightUp,
	MoveRightDown,
	Rest,
	PickUp,
	Search,
	Interact,
	Ascend,
	Descend,
};

class Player
{
public:
	Player();

	Action::Ptr getAction(sf::Keyboard::Key key);

private:
	void initializeKeys();
	void initializeActions();

private:
	std::unordered_map<sf::Keyboard::Key, Command> keyBinding;
	// std::unordered_map<Command, Action::Ptr> commandBinding;
	std::unordered_map<Command, std::function<Action::Ptr(bool)>> commandBinding;
};
