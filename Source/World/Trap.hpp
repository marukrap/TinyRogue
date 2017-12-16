#pragma once

#include "Entity.hpp"

#include <memory>

class Actor;

class Trap : public Entity
{
public:
	using Ptr = std::unique_ptr<Trap>;

public:
	enum Type
	{
		Door,
		Arrow,
		Sleep,
		Bear,
		Telep,
		Dart,
		Rust,
		Myst,
		TrapCount
	};

public:
	explicit Trap(Type type);
	
	bool isActive() const;

	void activate();
	void deactivate();

	void beTrapped(Actor& actor);

private:
	Type type;
	bool active = false;
};
