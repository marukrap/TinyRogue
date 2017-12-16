#pragma once

/*
TODO:
	Confuse: confuse 상태의 몬스터가 다른 몬스터를 공격하는지 테스트
	Protect: 아이템에 걸린 저주를 해제할지의 여부 (오리지널에서는 x)
*/

#include "../Item.hpp"

class Scroll : public Item
{
public:
	enum Type
	{
		Confuse,
		Map,
		Hold,
		Sleep,
		EnchantArmor,
		IdPotion, // TODO: ID 스크롤을 하나로
		IdScroll,
		IdWeapon,
		IdArmor,
		IdRingOrStick,
		Scare,
		FoodFind,
		Teleport,
		EnchantWeapon,
		Create,
		Remove,
		Aggravate,
		Protect,
		MaxScrolls
	};

public:
	using Item::Item;

	std::wstring getAName() const override;

	// read
	bool use(Actor& actor) override;

	void identify() override;
	bool isIdentified() const override;

	static void initNames(Rng& rng);

	static int pickOne(Rng& rng);
};
