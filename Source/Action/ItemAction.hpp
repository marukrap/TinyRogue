#pragma once

#include "Action.hpp"

class Item;

class PickUpAction : public Action
{
public:
	PickUpAction();
	explicit PickUpAction(Item& item);

	bool perform(Actor& actor) override;

private:
	Item* item = nullptr;
};

class UseAction : public Action
{
public:
	explicit UseAction(Item& item);

	bool perform(Actor& actor) override;

private:
	Item& item;
};

class DropAction : public Action
{
public:
	explicit DropAction(Item& item);
	
	bool perform(Actor& actor) override;

private:
	Item& item;
};

class ThrowAction : public Action
{
public:
	explicit ThrowAction(Item& item);
	
	bool perform(Actor& actor) override;

private:
	Item& item;
};