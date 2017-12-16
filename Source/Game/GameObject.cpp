#include "GameObject.hpp"
#include "../World/World.hpp"
#include "../States/StateStack.hpp"

#include <SFML/Audio/Sound.hpp>

#include <list>

namespace
{
	World* world = nullptr;
	Status* status = nullptr;
	MessageLog* messageLog = nullptr;
	ResourceManager* resources = nullptr;
	StateStack* stack = nullptr;
	std::list<sf::Sound> sounds;
}

void GameObject::setWorld(World& world)
{
	::world = &world;
}

void GameObject::setStatus(Status& status)
{
	::status = &status;
}

void GameObject::setLog(MessageLog& messageLog)
{
	::messageLog = &messageLog;
}

void GameObject::setResources(ResourceManager& resources)
{
	::resources = &resources;
}

void GameObject::setStack(StateStack& stack)
{
	::stack = &stack;
}

World& GameObject::getWorld() const
{
	return *world;
}

Level& GameObject::getLevel() const
{
	return world->getLevel();
}

Status& GameObject::getStatus() const
{
	return *status;
}

MessageLog& GameObject::getLog() const
{
	return *messageLog;
}

void GameObject::ascend()
{
	world->ascend();
}

void GameObject::descend()
{
	world->descend();
}

void GameObject::teleport(Actor& actor)
{
	/* wizard.c */
	// teleport: Bamf the hero someplace else

	sf::Vector2i pos;

	do
		pos = world->getLevel().findPassableTile();
	while (world->getLevel().actorAt(pos));

	actor.setPosition(pos);

	if (actor.getType() == Actor::Hero)
	{
		if (actor.hasFlag(Actor::IsHeld))
			actor.unhold();

		actor.removeEffect(Actor::NoMove);
	}

	playSound(SoundID::Teleport);
}

void GameObject::message(const std::wstring& str, const sf::Color& color) const
{
	messageLog->message(str, color);
}

void GameObject::message(const std::wstring& str, const std::vector<Word>& words, const sf::Color& color) const
{
	messageLog->message(str, words, color);
}

void GameObject::playSound(SoundID id)
{
	sounds.emplace_back(resources->getSound(id));
	sounds.back().setVolume(75);
	sounds.back().play();
}

void GameObject::removeStoppedSounds()
{
	sounds.remove_if([] (const sf::Sound& s)
	{
		return s.getStatus() == sf::Sound::Stopped;
	});
}

void GameObject::pushState(std::unique_ptr<State> state)
{
	stack->pushState(std::move(state));
}
