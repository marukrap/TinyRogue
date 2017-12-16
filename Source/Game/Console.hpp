#pragma once

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/Vertex.hpp>

#include <vector>
#include <array>

namespace sf
{
	class Font;
	class Glyph;
}

class Console : public sf::Drawable, public sf::Transformable
{
public:
	enum Layer
	{
		Background,
		TextLayer,
		Foreground,
		LayerCount
	};

public:
	Console(int width, int height, const sf::Font& font, int fontSize);

	void clear();
	void clear(Layer layer);

	void setChar(int x, int y, wchar_t ch, const sf::Color& color = sf::Color::White);
	void setString(int x, int y, const std::wstring& str, const sf::Color& color = sf::Color::White);

	void setColor(int x, int y, const sf::Color& color, Layer layer = Background);
	void setColor(int left, int top, int width, int height, const sf::Color& color, Layer layer = Background);

	void setColorA(int x, int y, sf::Uint8 alpha); // used for FOV

	const sf::Vector2i& getSize() const;
	const sf::Vector2f& getTileSize() const;

private:
	bool isInBounds(int x, int y) const;

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	const sf::Font& font;
	const sf::Texture& texture;
	const int fontSize;

	sf::Vector2i size;
	sf::Vector2f tileSize;
	sf::Vector2f origin;

	std::array<std::vector<sf::Vertex>, LayerCount> layers;
	std::array<const sf::Glyph*, 95> ascii;
};
