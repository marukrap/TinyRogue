#pragma once

#include "Console.hpp"
#include "Status.hpp"
#include "MessageLog.hpp"
#include "../States/StateStack.hpp"
#include "../Graphics/BloomEffect.hpp"
#include "../Utility/ResourceManager.hpp"

#include <SFML/Graphics/RenderWindow.hpp>

class Game
{
public:
	Game();

	void run();

private:
	void processInput();
	void update(sf::Time dt);
	void render();

	void loadResources();
	void initializeWindow(sf::VideoMode mode, sf::Uint32 style);
	void toggleFullscreen();

private:
	sf::RenderWindow window;
	sf::RenderTexture renderTexture;
	std::unique_ptr<Console> console;
	std::unique_ptr<Status> status;
	std::unique_ptr<MessageLog> log;
	StateStack stack;
	ResourceManager resources;
	BloomEffect bloomEffect;
	bool postEffect = true;
};
