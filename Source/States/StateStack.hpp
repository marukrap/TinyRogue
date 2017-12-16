#pragma once

#include "State.hpp"

#include <SFML/Window/Event.hpp>

#include <vector>

class StateStack
{
public:
	StateStack() = default;

	// NonCopyable
	StateStack(const StateStack&) = delete;
	StateStack& operator=(const StateStack&) = delete;

	void handleEvent(const sf::Event& event);
	void update(sf::Time dt);

	void drawBefore();
	void drawAfter();

	void pushState(State::Ptr state);
	void popState();
	void clearStates();

	bool isEmpty() const;

private:
	enum class Action
	{
		Push,
		Pop,
		Clear
	};

	void applyPendingChanges();

private:
	std::vector<State::Ptr> stack;
	std::vector<std::pair<Action, State::Ptr>> pendingList;
};
