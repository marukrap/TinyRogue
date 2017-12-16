#pragma once

#include "Actor.hpp"
#include "Item.hpp"
#include "Trap.hpp"
#include "../Map/Map.hpp"

#include <SFML/Graphics/Rect.hpp>

#include <functional>

class Rng;

class Level : public Map, public GameObject
{
public:
	using Ptr = std::unique_ptr<Level>;

public:
	explicit Level(Rng& rng, int width, int height);

	// TODO: Use template
	void attach(Actor::Ptr actor);
	Actor::Ptr detach(Actor& actor);

	void attach(Item::Ptr item);
	Item::Ptr detach(Item& item);

	void attach(Trap::Ptr trap);
	Trap::Ptr detach(Trap& trap);

	Actor* actorAt(const sf::Vector2i& pos) const;
	Item* itemAt(const sf::Vector2i& pos) const;
	Trap* trapAt(const sf::Vector2i& pos) const;

	void foreachActors(std::function<void(Actor&)> function);
	void foreachItems(std::function<void(Item&)> function);

	sf::Vector2i findPassableTile();
	sf::Vector2i findRandomDoor(const sf::Vector2i& pos);

	bool isInRoom(const sf::Vector2i& pos) const;
	bool lightRoom(const sf::Vector2i& pos);

	void setUpStairs(const sf::Vector2i& pos);
	void setDownStairs(const sf::Vector2i& pos);

	void removeDeadActors();

	// Used for hallucination effect
	void colorize();
	void repaint();

	void update(sf::Time dt);
	void draw(Console& console) override;
	void debugDraw(Console& console);
	void postProcess(Console& console);

private:
	Rng& rng;
	std::vector<Actor::Ptr> actors;
	std::vector<Item::Ptr> items;
	std::vector<Trap::Ptr> traps;

	sf::Vector2i upStairs;
	sf::Vector2i downStairs;

	std::vector<sf::IntRect> rooms;

	sf::Time elapsedTime;
	bool hallucinate = false;

	// REMOVE:
	friend class World;
	friend class Dungeon;
};
