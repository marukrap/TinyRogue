#pragma once

#include "../Item.hpp"

class Stick : public Item
{
public:
	enum Type
	{
		Light,
		Invisibility,
		Lightning,
		Fire,
		Cold,
		Polymorph,
		Missile,
		Haste,
		Slow,
		Drain,
		Nothing,
		TeleportAway,
		TeleportTo,
		Cancel,
		MaxSticks
	};

public:
	using Item::Item;

	std::wstring getAName() const override;

	std::string getDamage() const override;
	std::string getDamageThrown() const override;

	// zap
	bool use(Actor& actor) override;

	// void enchant() override;
	
	void identify() override;

	static void initMaterials(Rng& rng);
	
	static Item::Ptr createStick(int which, int charges = 0);

	static int pickOne(Rng& rng);

private:
	int charges = 0;
};
