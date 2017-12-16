#pragma once

#include "../Game/GameObject.hpp"

#include <memory> // unique_ptr

class Actor;

class Action : public GameObject
{
public:
	using Ptr = std::unique_ptr<Action>;

public:
	virtual ~Action() = default;

	virtual bool perform(Actor& actor) = 0;
};
