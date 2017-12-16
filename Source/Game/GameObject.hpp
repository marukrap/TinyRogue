#pragma once

// TODO: Rework

#include "MessageLog.hpp"
#include "../Utility/ResourceManager.hpp"

class World;
class Level;
class Status;
class Actor;
class State;
class StateStack;

class GameObject
{
public:
	static void setWorld(World& world);
	static void setStatus(Status& status);
	static void setLog(MessageLog& messageLog);
	static void setResources(ResourceManager& resources);
	static void setStack(StateStack& stack);

protected:
	World& getWorld() const;
	Level& getLevel() const;
	Status& getStatus() const;
	MessageLog& getLog() const;

	// NOTE: Functions forward to World
	void ascend();
	void descend();
	void teleport(Actor& actor);
	//

	// TODO: Perfect forwarding
	void message(const std::wstring& str, const sf::Color& color = sf::Color::White) const;
	void message(const std::wstring& str, const std::vector<Word>& words, const sf::Color& color = sf::Color::White) const;

	void playSound(SoundID id);
	void removeStoppedSounds();

	void pushState(std::unique_ptr<State> state);
};
