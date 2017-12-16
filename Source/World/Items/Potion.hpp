#pragma once

/*
TODO:
	Hallucinate: 관련 코드 최적화/간소화
	Blind: 실명 상태인 경우 자동이동 처리(함정 등)
	Levitate: 전투 관련 처리
	Throw a potion (Epyx Rogue)
*/

#include "../Item.hpp"

class Potion : public Item
{
public:
	enum Type
	{
		Confuse,
		Hallucinate,
		Poison,
		Strength,
		SeeInvisible,
		Healing,
		MonsterFind,
		MagicFind,
		RaiseLevel,
		ExtraHeal,
		Haste,
		Restore,
		Blind,
		Levitate,
		MaxPotions
	};

public:
	using Item::Item;

	std::wstring getAName() const override;

	// quaff
	bool use(Actor& actor) override;
	
	void identify() override;
	bool isIdentified() const override;

	static void initColors(Rng& rng);

	static std::wstring randomColor(); // Rainbow
	static std::wstring pickColor(Actor& actor, const std::wstring& color);

	static int pickOne(Rng& rng);

private:
	void quaff(Actor& actor, int type, bool knowit, int flag = 0, int time = 0);
};
