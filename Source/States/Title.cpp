#include "Title.hpp"
#include "Play.hpp"
#include "StateStack.hpp"
#include "../Game/Console.hpp"
#include "../Graphics/BloomEffect.hpp"
#include "../Utility/Utility.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RenderTexture.hpp>

Title::Title()
{
	const sf::Vector2i mapSize(80, 25);

	sf::Texture texture;
	texture.loadFromFile("Images/title.png");

	sf::Sprite sprite(texture);
	sprite.setScale(static_cast<float>(mapSize.x) / texture.getSize().x, static_cast<float>(mapSize.y) / texture.getSize().y);

	sf::RenderTexture renderTexture;
	renderTexture.create(mapSize.x, mapSize.y);
	renderTexture.clear();
	renderTexture.draw(sprite);
	renderTexture.display();

	sf::Image image = renderTexture.getTexture().copyToImage();
	image.createMaskFromColor(sf::Color::Black);

	console->clear();

	for (int y = 0; y < mapSize.y; ++y)
		for (int x = 0; x < mapSize.x; ++x)
		{
			// console->setChar(x, y, L'#', image.getPixel(x, y));
			console->setChar(x, y, randomInt(L'A', L'Z'), image.getPixel(x, y));
		}

	BloomEffect::hallucination = true;

	std::wstring str = L"PRESS ANY KEY TO CONTINUE";
	const int status = 20;
	int size = ((mapSize.x - status) - str.size() - status) / 2;
	str = std::wstring(size, L' ') + str;
	message(L"");
	message(L"");
	message(str);

	// TODO: Menus?
}

void Title::handleKeyboard(sf::Keyboard::Key key)
{
	switch (key)
	{
	// case sf::Keyboard::Escape:
	case sf::Keyboard::LControl:
	case sf::Keyboard::LShift:
	case sf::Keyboard::LAlt:
	case sf::Keyboard::LSystem:
	case sf::Keyboard::RControl:
	case sf::Keyboard::RShift:
	case sf::Keyboard::RAlt:
	case sf::Keyboard::RSystem:
	case sf::Keyboard::Menu:
		return;
	}

	BloomEffect::hallucination = false;

	getLog().popBack();
	getLog().popBack();
	getLog().popBack();

	stack->popState();
	stack->pushState(std::make_unique<Play>());
}

void Title::update(sf::Time dt)
{
}
