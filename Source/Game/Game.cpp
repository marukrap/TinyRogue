#include "Game.hpp"
#include "../States/Title.hpp"
#include "../Utility/WindowsHelper.hpp"

#include <SFML/Window/Event.hpp>

#include <algorithm> // max
#include <iostream>

namespace
{
	const std::wstring title = L"Tiny Rogue";
	const sf::Vector2i mapSize(80, 25);

	const sf::Vector2i statusSize(20, Status::TypeCount);
	const sf::Vector2i logSize(mapSize.x - statusSize.x - 2, 5);

	const int bottomMargin = 5;

	std::string fps;
	std::string mousePos;
}

Game::Game()
	: bloomEffect(resources)
{
	loadResources();

	console = std::make_unique<Console>(mapSize.x, mapSize.y, resources.getFont(FontID::Console), 20);
	sf::Vector2i size = console->getSize();
	sf::Vector2f tileSize = console->getTileSize();

	status = std::make_unique<Status>(statusSize.x, statusSize.y, resources.getFont(FontID::UI), 20, tileSize);
	status->setPosition(0.f, size.y * tileSize.y);

	log = std::make_unique<MessageLog>(logSize.x, logSize.y, resources.getFont(FontID::UI), 20, tileSize);
	log->setPosition((statusSize.x + 1) * tileSize.x, size.y * tileSize.y);

	size.y += std::max(statusSize.y, logSize.y);

	unsigned int width = static_cast<unsigned int>(size.x * tileSize.x);
	unsigned int height = static_cast<unsigned int>(size.y * tileSize.y) + bottomMargin;
	std::cout << "Window Size: " << width << " x " << height << '\n';

	initializeWindow(sf::VideoMode(width, height), sf::Style::Close);

	renderTexture.create(width, height);

	GameObject::setStatus(*status);
	GameObject::setLog(*log);
	GameObject::setResources(resources);
	GameObject::setStack(stack);

	State::initialize(stack, *console);
	stack.pushState(std::make_unique<Title>());
}

void Game::run()
{
	static const sf::Time timePerFrame = sf::seconds(1.f / 60.f);

	sf::Clock clock;
	sf::Time timeSinceLastUpdate;

	while (window.isOpen())
	{
		sf::Time dt = clock.restart();
		timeSinceLastUpdate += dt;

		if (timeSinceLastUpdate >= timePerFrame)
		{
			timeSinceLastUpdate -= timePerFrame;

			processInput();
			update(timePerFrame);
		}

#ifdef _DEBUG
		// FPS Counter
		static sf::Time elapsedTime;
		static std::size_t numFrames = 0;

		elapsedTime += dt;
		numFrames += 1;

		if (elapsedTime >= sf::seconds(1.f))
		{
			fps = "FPS: " + std::to_string(numFrames);
			window.setTitle(fps + mousePos);
			elapsedTime -= sf::seconds(1.f);
			numFrames = 0;
		}
#endif

		render();
	}
}

void Game::processInput()
{
	sf::Event event;

	while (window.pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
			window.close();

		else if (event.type == sf::Event::KeyPressed)
		{
			switch (event.key.code)
			{
			case sf::Keyboard::F9:
				postEffect = !postEffect;
				break;
			
			// case sf::Keyboard::F10:
				// toggleFullscreen();
				// break;
			}
		}

#ifdef _DEBUG
		else if (event.type == sf::Event::MouseMoved)
		{
			int x = event.mouseMove.x / static_cast<int>(console->getTileSize().x);
			int y = event.mouseMove.y / static_cast<int>(console->getTileSize().y);

			mousePos = " Mouse: " + std::to_string(x) + ',' + std::to_string(y);
			window.setTitle(fps + mousePos);
		}
#endif

		stack.handleEvent(event);
	}
}

void Game::update(sf::Time dt)
{
	stack.update(dt);
}

void Game::render()
{
	// TODO: Reduce draw calls (Console)

	window.clear();

	stack.drawBefore();

	if (postEffect && PostEffect::isSupported())
	{
		renderTexture.clear();
		renderTexture.draw(*console);
		renderTexture.display();
		bloomEffect.apply(renderTexture, window);
	}

	stack.drawAfter();

	window.draw(*console);
	window.draw(*status);
	window.draw(*log);

	window.display();
}

void Game::loadResources()
{
	resources.loadFont(FontID::Console, "Fonts/VT220-mod.ttf");
	resources.loadFont(FontID::UI, "Fonts/VT220-mod.ttf");

	// Sound effects from Donnie Russell's Tile Rogue
	resources.loadSound(SoundID::Armor, "Sound/armor.wav");
	resources.loadSound(SoundID::Bell, "Sound/bell.wav");
	resources.loadSound(SoundID::Confusion, "Sound/confusion.wav");
	resources.loadSound(SoundID::Eat, "Sound/eat.wav");
	resources.loadSound(SoundID::Experience, "Sound/experience.wav");
	resources.loadSound(SoundID::Fire, "Sound/fire.wav");
	resources.loadSound(SoundID::Found, "Sound/found.wav");
	resources.loadSound(SoundID::Gold, "Sound/gold.wav");
	resources.loadSound(SoundID::Ice, "Sound/ice.wav");
	resources.loadSound(SoundID::MonsterDie, "Sound/monsterdie.wav");
	resources.loadSound(SoundID::MonsterHit, "Sound/monsterhit.wav");
	resources.loadSound(SoundID::MonsterMiss, "Sound/monstermiss.wav");
	resources.loadSound(SoundID::RogueDie, "Sound/roguedie.wav");
	resources.loadSound(SoundID::RogueHit, "Sound/roguehit.wav");
	resources.loadSound(SoundID::RogueMiss, "Sound/roguemiss.wav");
	resources.loadSound(SoundID::Stairs, "Sound/stairs.wav");
	resources.loadSound(SoundID::Teleport, "Sound/teleport.wav");
	resources.loadSound(SoundID::Tick, "Sound/tick.wav");
	resources.loadSound(SoundID::Trap, "Sound/trap.wav");
	resources.loadSound(SoundID::Win, "Sound/win.wav");
	resources.loadSound(SoundID::Zap, "Sound/zap.wav");
}

void Game::initializeWindow(sf::VideoMode mode, sf::Uint32 style)
{
	window.create(mode, title, style);
	// window.setFramerateLimit(30);
	// window.setVerticalSyncEnabled(true);

	centerWindow(window.getSystemHandle());
}

void Game::toggleFullscreen()
{
	static bool fullscreen = false;

	fullscreen = !fullscreen;

	sf::VideoMode mode;
	sf::Uint32 style;

	if (fullscreen)
	{
		mode = sf::VideoMode::getDesktopMode();
		style = sf::Style::None;
	}

	else
	{
		sf::Vector2i size = console->getSize();
		sf::Vector2f tileSize = console->getTileSize();

		size.y += std::max(statusSize.y, logSize.y);

		mode.width = static_cast<unsigned int>(size.x * tileSize.x);
		mode.height = static_cast<unsigned int>(size.y * tileSize.y) + bottomMargin;
		style = sf::Style::Close;
	}

	initializeWindow(mode, style);
}
