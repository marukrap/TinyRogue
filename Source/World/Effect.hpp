#pragma once

#include "../Game/GameObject.hpp"

class Actor;

class Effect : public GameObject
{
public:
	Effect(int flag, int duration); // Actor::Flags

	int getFlag() const;
	int getDuration() const;

	bool isFinished() const;
	void lengthen(int duration);

	void start(Actor& actor);
	void update(Actor& actor);
	void end(Actor& actor);

private:
	int flag;
	int duration = -1;
};
