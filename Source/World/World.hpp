#pragma once

#include "Level.hpp"
#include "../Game/Player.hpp"
#include "../Map/Dungeon.hpp"
#include "../Fov/Fov.hpp"
#include "../Utility/Rng.hpp"
#include "../States/State.hpp"

#include <SFML/Window/Keyboard.hpp>
#include <SFML/System/Time.hpp>

class Console;
class Status;
class ActorPlayer;

class World : public GameObject
{
public:
	explicit World(Console& console);

	void handleKeyboard(sf::Keyboard::Key key);
	void update(sf::Time dt);
	void draw();

	int getDepth() const;
	Level& getLevel() const;
	ActorPlayer& getPlayerActor() const;

	bool isVisible(const Actor& actor) const;
	bool isVisible(const Item& item) const;

	void ascend();
	void descend();

	void aggravate();

	void endPlayerTurn();
	void updateEnemies();
	void updateStatus();

	// Fov
	void computeFov();
	void reloadFov();
	void setFovRange(float range);
	float getFovRange() const;
	void resetFovRange();

private:
	Console& console;
	Rng rng;

	// Level
	Dungeon dungeonGenerator;
	std::vector<Level::Ptr> levels;
	Level* currentLevel = nullptr;
	std::size_t currentDepth = 1;

	// Fov
	Fov fov;
	float fovRange = 0.f;

	// Player
	Player player;
	ActorPlayer* playerActor = nullptr;

	//
	bool debugDraw = false;
};
