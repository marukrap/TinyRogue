#pragma once

#include "../Item.hpp"

class Weapon : public Item
{
public:
	enum Type
	{
		Mace,
		Sword,
		Bow,
		Arrow,
		Dagger,
		TwoSword,
		Dart,
		Shuriken, // Fix: Shiraken > Shuriken (typo?)
		Spear,
		Flame, // Fake entry for dragon's breath
		MaxWeapons = Flame
	};

public:
	using Item::Item;

	std::wstring getName() const override;
	std::wstring getAName() const override;

	int getLaunch() const override;

	std::string getDamage() const override;
	std::string getDamageThrown() const override;
	int getHitPlus() const override;
	int getDamagePlus() const override;

	// wield/unwield
	bool use(Actor& actor) override;

	void enchant() override;

	Item::Ptr detachOne() override;

	static Item::Ptr createWeapon(int which, int hitPlus = 0, int damagePlus = 0);
	static Item::Ptr createBolt(wchar_t ch, const sf::Color& color, const std::wstring& name, const std::string& damage, int hitPlus, int damagePlus, bool isMissile = false);
	
	static int pickOne(Rng& rng);

private:
	// std::string damage;			// Damage if used like sword
	// std::string damageThrown;	// Damage if thrown
	int hitPlus;				// Plusses to hit
	int damagePlus;				// Plusses to damage
};
