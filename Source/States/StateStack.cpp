#include "StateStack.hpp"

void StateStack::handleEvent(const sf::Event& event)
{
	// for (auto it = stack.rbegin(); it != stack.rend(); ++it)
		// (*it)->handleEvent(event);

	if (event.type == sf::Event::KeyPressed && !stack.empty())
		stack.back()->handleKeyboard(event.key.code);

	applyPendingChanges();
}

void StateStack::update(sf::Time dt)
{
	for (auto it = stack.rbegin(); it != stack.rend(); ++it)
		(*it)->update(dt);

	applyPendingChanges();
}

void StateStack::drawBefore()
{
	for (auto& state : stack)
		state->drawBefore();
}

void StateStack::drawAfter()
{
	for (auto& state : stack)
		state->drawAfter();
}

void StateStack::pushState(State::Ptr state)
{
	pendingList.emplace_back(Action::Push, std::move(state));
}

void StateStack::popState()
{
	pendingList.emplace_back(Action::Pop, nullptr);
}

void StateStack::clearStates()
{
	pendingList.emplace_back(Action::Clear, nullptr);
}

bool StateStack::isEmpty() const
{
	return stack.empty();
}

void StateStack::applyPendingChanges()
{
	for (auto& change : pendingList)
	{
		switch (change.first)
		{
		case Action::Push:
			stack.emplace_back(std::move(change.second));
			break;

		case Action::Pop:
			stack.pop_back();
			break;

		case Action::Clear:
			stack.clear();
			break;
		}
	}

	pendingList.clear();
}
