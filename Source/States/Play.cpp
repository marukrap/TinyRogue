#include "Play.hpp"
#include "Help.hpp"
#include "Inventory.hpp"
#include "StateStack.hpp"
#include "../World/ActorPlayer.hpp"
#include "../Action/ItemAction.hpp"

Play::Play()
	: world(*console)
{
	// GameObject::setWorld(world);
}

void Play::handleKeyboard(sf::Keyboard::Key key)
{
	ActorPlayer& actor = world.getPlayerActor();

	if (actor.hasFlag(Actor::NoCommand))
		return;

	switch (key)
	{
	case sf::Keyboard::F1:
		stack->pushState(std::make_unique<Help>());
		break;

	case sf::Keyboard::Slash:
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift))
			stack->pushState(std::make_unique<Help>());
		break;

	case sf::Keyboard::I:
	case sf::Keyboard::Insert:
	case sf::Keyboard::Numpad0:
		stack->pushState(std::make_unique<Inventory>(actor));
		break;

	case sf::Keyboard::A:
		if (!actor.isDead())
			stack->pushState(std::make_unique<Inventory>(actor, Inventory::Apply));
		break;

	case sf::Keyboard::D:
		if (!actor.isDead())
			stack->pushState(std::make_unique<Inventory>(actor, Inventory::Drop));
		break;

	case sf::Keyboard::F:
		if (!actor.isDead() && actor.thrownItem && actor.thrownItem->dropCheck(actor))
			std::make_unique<ThrowAction>(*actor.thrownItem)->perform(actor);
		// else
			// message(L"You don't have anything appropriate", Color::White);
		break;

	case sf::Keyboard::T:
		if (!actor.isDead())
			stack->pushState(std::make_unique<Inventory>(actor, Inventory::Throw));
		break;
	}

	world.handleKeyboard(key);
}

void Play::update(sf::Time dt)
{
	world.update(dt);
}

void Play::drawBefore()
{
	world.draw();
}
