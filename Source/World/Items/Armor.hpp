#pragma once

#include "../Item.hpp"

class Armor : public Item
{
public:
	enum Type
	{
		Leather,
		RingMail,
		StuddedLeather,
		ScaleMail,
		ChainMail,
		SplintMail,
		BandedMail,
		PlateMail,
		MaxArmors
	};

public:
	using Item::Item;

	std::wstring getName() const override;
	std::wstring getAName() const override;

	int getArmor() const override;

	// wear/take off
	bool use(Actor& actor) override;

	void enchant() override;
	void degrade() override;

	static Item::Ptr createArmor(int which, int ac = 0);
	
	static int pickOne(Rng& rng);

private:
	int armor = 0; // Armor protection
};
