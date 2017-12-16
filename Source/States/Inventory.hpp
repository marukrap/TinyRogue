#pragma once

#include "State.hpp"

#include <vector>

class Actor;
class Item;

class Inventory : public State
{
public:
	enum Mode
	{
		Normal,
		Apply,
		Drop,
		Throw,
		IdPotion,
		IdScroll,
		IdWeapon,
		IdArmor,
		IdRingOrStick,
	};

public:
	Inventory(Actor& actor, Mode mode = Normal);

	void handleKeyboard(sf::Keyboard::Key key) override;
	void update(sf::Time dt) override;
	void drawAfter() override;

private:
	bool isClassic() const;

	void use();
	void drop();
	void hurl(); // throw

private:
	Actor& actor;
	Mode mode;

	std::vector<Item*> items;
	int selected = -1;

	std::vector<std::wstring> commands;
	int currentCommand = -1;
};
