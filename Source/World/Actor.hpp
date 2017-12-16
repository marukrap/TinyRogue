#pragma once

#include "Effect.hpp"
#include "Entity.hpp"

#include "../Utility/Direction.hpp"

#include <memory>
#include <vector>
#include <array>

class Item;
class Rng;

class Actor : public Entity
{
public:
	using Ptr = std::unique_ptr<Actor>;

	enum Type
	{
		Hero,
		Ally,
		Enemy,
		Neutral,
	};

	enum Slot
	{
		Weapon,
		Armor,
		LeftRing,
		RightRing,
		MaxSlot
	};

	enum Resist
	{
		Poison,
		Magic,
	};

	// TODO: Better names, remove 'Is' prefix
	enum Flags
	{
		None			= 0,
		CanConfuse		= 1 << 0,	// creature can confuse
		CanSee			= 1 << 1,	// creature can see invisible creatures
		IsBlind			= 1 << 2,	// creature is blind
		IsCancelled		= 1 << 3,	// creature has special qualities cancelled
		IsLevitating	= 1 << 4,	// hero is levitating
		IsFound			= 1 << 5,	// creature has been seen (used for objects)
		IsGreed			= 1 << 6,	// creature runs to protect gold
		IsHasted		= 1 << 7,	// creature has been hastened
		// IsTarget		= 1 << 8,	// creature is the target of an 'f' command
		IsHeld			= 1 << 9,	// creature has been held
		IsConfused		= 1 << 10,	// creature is confused
		IsInvisible		= 1 << 11,	// creature is invisible
		IsMean			= 1 << 12,	// creature can wake when player enters room
		IsHallucinating	= 1 << 13,	// hero is on acid trip
		IsRegen			= 1 << 14,	// creature can regenerate
		IsRunning		= 1 << 15,	// creature is running at the player
		SeeMonster		= 1 << 16,	// hero can detect unseen monsters
		IsFly			= 1 << 17,	// creature can fly
		IsSlowed		= 1 << 18,	// creature has been slowed

		NoMove			= 1 << 19,	// Number of turns held in place
		NoCommand		= 1 << 20,	// Number of turns asleep
	};

	struct Stats
	{
		int exp;			// Experience
		int level;			// Level of mastery
		int armor;			// Armor class
		std::string damage;	// String describing damage done
		int hp = 1;			// Hit points
		int maxhp = 1;		// Max hit points
		int str = 10;		// Strength
		int maxstr = 10;	// Max Strength
	};

public:
	explicit Actor(wchar_t ch);
	
	std::wstring getAName() const override;
	std::wstring getTheName() const override;
	
	const sf::Color& getColor() const override;
	
	Type getType() const;
	void setDisguise(wchar_t ch);
	wchar_t getDisguise() const;

	// Flags
	bool hasFlag(Flags flag) const;
	void addFlag(Flags flag);
	void removeFlag(Flags flag);

	// Stats
	int getExp() const;
	int getLevel() const;
	int getHp() const;
	int getMaxHp() const;
	int getStr() const;
	int getMaxStr() const;
	int getArmor() const;
	std::string getDamage() const;

	void gainExp(int exp);
	void raiseLevel();
	void lowerLevel();

	virtual void takeDamage(int points);
	void restoreHp(int points);
	void changeMaxHp(int points);

	bool isDead() const;
	bool isMarkedForRemoval() const;

	void changeStr(int amount);
	bool restoreStr();

	void kill();
	void killed(Actor& target);
	void death(const std::wstring& cause);

	virtual void attack(Actor& target);
	
	void startRun();
	void potionHeal(bool extra = false);

	// Used for venus flytrap
	void engulf();
	void unhold();

	// Inventory
	bool checkGroup(Item& item) const;
	Item* addPack(std::unique_ptr<Item> item);
	std::unique_ptr<Item> unpack(Item& item, bool hurl = false);
	int getNumItems() const;

	// Equipment
	void setEquipment(Slot slot, Item* item);
	Item* getEquipment(Slot slot) const;
	bool isEquipment(Slot slot, int which) const;
	bool hasRing(int ringType) const;

	// Effect
	void addEffect(Flags flag, int duration);
	void removeEffect(Flags flag);
	void updateEffects();

	int getEffectDuration(Flags flag) const;

	bool resistCheck(int which) const;
	bool throwCheck(int which) const;

	bool levitCheck() const;
	void rustArmor();

	void repaint();

	void draw(Console& console) override;

	static Actor::Ptr createMonster(Rng& rng, int depth, bool wander);
	static Actor::Ptr createMonster(int depth, bool wander);
	static Actor::Ptr createMonster(int type, int depth);

	std::wstring chooseStr(const std::wstring& str1, const std::wstring& str2);

protected:
	Type type;
	// int turn;			// If slowed, is it a turn to move
	wchar_t disguise;	// What mimic looks like
	// int oldch;			// Character that was where it was
	// sf::Vector2i dest;	// Where it is running to
	int flags = 0;		// State word
	Stats stats;		// Physical description

	std::array<Item*, MaxSlot> equipment;
	std::vector<std::unique_ptr<Effect>> effects;

// REMOVE:
public:
	sf::Vector2i destination = sf::Vector2i(-1, -1);
	Direction lastDir;
	int turnsWaited = 0;

	int purse = 0; // How much gold he has

	static const int MaxPackSize = 23;
	std::vector<std::unique_ptr<Item>> pack;

	// HACK: Used for haste/slow
	int ntimes = 1;
	bool turn = true;

	// HACK: Used for throw
	Item* thrownItem = nullptr;
};
