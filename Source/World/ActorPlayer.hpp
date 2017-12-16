#pragma once

#include "Actor.hpp"

#include <array>

class Item;

class ActorPlayer : public Actor
{
public:
	using Ptr = std::unique_ptr<ActorPlayer>;

public:
	using Actor::Actor;
	
	void attack(Actor& target) override;
	void takeDamage(int points) override;

	std::wstring getHungryState() const;
	void eatFood();

	// UNDONE:
	void setRunning(bool flag);
	bool isRunning() const;
	void run();
	//

	void revive();

	// daemons
	void digest();
	void rest();

	static ActorPlayer::Ptr createActor();

private:
	bool amulet = false;	// He found the amulet
	bool running = false;	// True if player is running
	int quiet = 0;			// Number of quiet turns
	int foodLeft = 0;		// Amount of food in hero's stomach
};
