#pragma once

#include "Action.hpp"
#include "../Utility/Direction.hpp"

class WalkAction : public Action
{
public:
	WalkAction(int dx, int dy);
	explicit WalkAction(const Direction& dir);

	bool perform(Actor& actor) override;

private:
	Direction dir;
};

class RunAction : public Action
{
public:
	RunAction(int dx, int dy);
	explicit RunAction(const Direction& dir);

	bool perform(Actor& actor) override;

// private:
public:
	Direction dir;
};

class AscendAction : public Action
{
public:
	bool perform(Actor& actor) override;
};

class DescendAction : public Action
{
public:
	bool perform(Actor& actor) override;
};

class SearchAction : public Action
{
public:
	bool perform(Actor& actor) override;
};

class InteractAction : public Action
{
public:
	bool perform(Actor& actor) override;
};
