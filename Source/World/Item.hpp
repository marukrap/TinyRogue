#pragma once

#include "Entity.hpp"

#include <memory>
#include <vector>

class Actor;
class Rng;

enum class ItemType
{
	Potion,
	Scroll,
	Food,
	Weapon,
	Armor,
	Ring,
	Stick,
	Gold,
	Amulet,
	TypeCount,
};

class Item : public Entity
{
public:
	using Ptr = std::unique_ptr<Item>;

	enum Flags
	{
		None		= 0,
		IsCursed	= 1 << 0, // object is cursed
		IsKnow		= 1 << 1, // player knows details about the object
		IsMissile	= 1 << 2, // object is a missile type
		IsMany		= 1 << 3, // object comes in groups
		IsProtected	= 1 << 4, // armor is permanently protected
		IsFound		= 1 << 5, // 
		IsDetected	= 1 << 6, // 
	};

	struct Info
	{
		std::wstring name;
		// std::wstring guess;
		int prob;
		int worth;
		bool know = false;
	};

public:
	explicit Item(ItemType type);

	std::wstring getAName() const override;

	ItemType getType() const;

	bool hasFlag(Flags flag) const;
	void addFlag(Flags flag);
	void removeFlag(Flags flag);

	void setCount(int count);
	int getCount() const;
	void addCount(int count);
	void reduceCount(int count);

	int getWhich() const;
	int getGroup() const;

	virtual int getLaunch() const;
	
	virtual std::string getDamage() const;
	virtual std::string getDamageThrown() const;
	virtual int getHitPlus() const;
	virtual int getDamagePlus() const;

	virtual int getArmor() const;

	virtual int getRingPower() const;

	bool isSame(ItemType type, int which) const;
	bool isSame(Item& item) const;

	virtual bool use(Actor& actor);
	void eat(Actor& actor);

	virtual void enchant();
	virtual void degrade();

	virtual int ringEat() const;

	virtual void identify();
	virtual bool isIdentified() const;

	bool isMagic() const;
	bool dropCheck(Actor& actor) const;

	void repaint();

	static void initItems(Rng& rng);

	virtual Item::Ptr detachOne();

	static Item::Ptr createItem(ItemType type, int which);
	static Item::Ptr createItem(Rng& rng, int& noFood);
	static Item::Ptr createGold(int value);

	static std::wstring getFruit();
	static wchar_t getRandomChar();

protected:
	static int pickOne(Rng& rng, std::vector<Info>& items);

protected:
	ItemType type;
	int count = 1;	// Count for plural objects
	int which = -1;	// Which object of a type it is
	int flags = 0;	// Information about objects
	int group = 0;  // Group number for this object

	static int Group;
};
