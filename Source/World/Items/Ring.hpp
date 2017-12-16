#pragma once

/*
TODO:
	Search: ¹Ì±¸Çö
*/

#include "../Item.hpp"

class Ring : public Item
{
public:
	enum Type
	{
		Protect,
		AddStrength,
		SustainStrength,
		Search,
		SeeInvisible,
		Nothing,
		Aggravate,
		AddHit,
		AddDamage,
		Regenerate,
		Digest,
		Teleport,
		Stealth,
		MaintainArmor,
		MaxRings
	};

public:
	using Item::Item;

	std::wstring getName() const override;
	std::wstring getAName() const override;

	int getRingPower() const override;

	// put on/remove
	bool use(Actor& actor) override;

	void enchant() override;
	int ringEat() const override;

	void identify() override;

	static void initStones(Rng& rng);

	static Item::Ptr createRing(int which, int power = 0);
	
	static int pickOne(Rng& rng);

private:
	void putOn(Actor& actor, int slot);
	void remove(Actor& actor, int slot);

private:
	int power = 0;
};
