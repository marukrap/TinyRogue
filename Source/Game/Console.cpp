#include "Console.hpp"

#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include <cmath> // floor
#include <algorithm> // for_each
#include <iostream>

Console::Console(int width, int height, const sf::Font& font, int fontSize)
	: font(font)
	, texture(font.getTexture(fontSize))
	, fontSize(fontSize)
	, size(width, height)
{
	const_cast<sf::Texture&>(texture).setSmooth(false);

	tileSize.x = font.getGlyph(L'A', fontSize, false).advance;
	tileSize.y = font.getLineSpacing(fontSize);
	std::cout << "Console TileSize: " << tileSize.x << " x " << tileSize.y << '\n';

	sf::FloatRect bounds = sf::Text(L'A', font, fontSize).getLocalBounds();
	float topSpace = bounds.top;
	float bottomSpace = tileSize.y - (bounds.top + bounds.height);
	origin.y = std::floor((topSpace - bottomSpace) / 2.f);
	std::cout << "Console Origin: " << origin.x << ", " << origin.y << '\n';

	// HACK: for VT220-mod.ttf
	tileSize.y += 1;
	origin.y += 1;
	//

	for (auto& layer : layers)
	{
		layer.resize(width * height * 4);

		for (int y = 0; y < height; ++y)
			for (int x = 0; x < width; ++x)
			{
				sf::Vertex* quad = &layer[(x + y * width) * 4];

				quad[0] = sf::Vertex(sf::Vector2f((x + 0) * tileSize.x, (y + 0) * tileSize.y), sf::Color::Transparent);
				quad[1] = sf::Vertex(sf::Vector2f((x + 1) * tileSize.x, (y + 0) * tileSize.y), sf::Color::Transparent);
				quad[2] = sf::Vertex(sf::Vector2f((x + 1) * tileSize.x, (y + 1) * tileSize.y), sf::Color::Transparent);
				quad[3] = sf::Vertex(sf::Vector2f((x + 0) * tileSize.x, (y + 1) * tileSize.y), sf::Color::Transparent);
			}
	}

	// Preload ascii glyphs from ' ' to '~'
	for (std::size_t i = 0; i < ascii.size(); ++i)
		ascii[i] = &font.getGlyph(i + 32, fontSize, false);
}

void Console::clear()
{
	for (int i = 0; i < LayerCount; ++i)
		clear(static_cast<Layer>(i));
}

void Console::clear(Layer layer)
{
	std::for_each(layers[layer].begin(), layers[layer].end(), [] (auto& v) { v.color.a = 0; });
}

void Console::setChar(int x, int y, wchar_t ch, const sf::Color& color)
{
	if (!isInBounds(x, y))
		return;

	const sf::Glyph* glyph = nullptr;

	if (ch >= 32 && ch < 127)
		glyph = ascii[ch - 32];
	else
		glyph = &font.getGlyph(ch, fontSize, false);

	float x1 = glyph->bounds.left;
	float y1 = glyph->bounds.top + fontSize - origin.y;
	float x2 = glyph->bounds.left + glyph->bounds.width;
	float y2 = glyph->bounds.top + glyph->bounds.height + fontSize - origin.y;

	float u1 = static_cast<float>(glyph->textureRect.left);
	float v1 = static_cast<float>(glyph->textureRect.top);
	float u2 = static_cast<float>(glyph->textureRect.left + glyph->textureRect.width);
	float v2 = static_cast<float>(glyph->textureRect.top + glyph->textureRect.height);

	sf::Vertex* quad = &layers[TextLayer][(x + y * size.x) * 4];

	quad[0] = sf::Vertex(sf::Vector2f(x * tileSize.x + x1, y * tileSize.y + y1), color, sf::Vector2f(u1, v1));
	quad[1] = sf::Vertex(sf::Vector2f(x * tileSize.x + x2, y * tileSize.y + y1), color, sf::Vector2f(u2, v1));
	quad[2] = sf::Vertex(sf::Vector2f(x * tileSize.x + x2, y * tileSize.y + y2), color, sf::Vector2f(u2, v2));
	quad[3] = sf::Vertex(sf::Vector2f(x * tileSize.x + x1, y * tileSize.y + y2), color, sf::Vector2f(u1, v2));
}

void Console::setString(int x, int y, const std::wstring& str, const sf::Color& color)
{
	int dx = 0;
	int dy = 0;

	for (wchar_t ch : str)
	{
		if (ch == L'\t')
		{
			for (int i = 0; i < 4; ++i)
				setChar(x + dx++, y + dy, L' ', color);
		}

		else if (ch == L'\n')
		{
			dx = 0;
			dy += 1;
		}

		else
			setChar(x + dx++, y + dy, ch, color);
	}
}

void Console::setColor(int x, int y, const sf::Color& color, Layer layer)
{
	if (!isInBounds(x, y))
		return;

	sf::Vertex* quad = &layers[layer][(x + y * size.x) * 4];

	quad[0].color = color;
	quad[1].color = color;
	quad[2].color = color;
	quad[3].color = color;
}

void Console::setColor(int left, int top, int width, int height, const sf::Color& color, Layer layer)
{
	for (int y = top; y < top + height; ++y)
		for (int x = left; x < left + width; ++x)
			setColor(x, y, color, layer);
}

void Console::setColorA(int x, int y, sf::Uint8 alpha)
{
	if (!isInBounds(x, y))
		return;

	sf::Vertex* quad = &layers[TextLayer][(x + y * size.x) * 4];

	quad[0].color.a = alpha;
	quad[1].color.a = alpha;
	quad[2].color.a = alpha;
	quad[3].color.a = alpha;
}

const sf::Vector2i& Console::getSize() const
{
	return size;
}

const sf::Vector2f& Console::getTileSize() const
{
	return tileSize;
}

bool Console::isInBounds(int x, int y) const
{
	return x >= 0 && x < size.x && y >= 0 && y < size.y;
}

void Console::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();

	for (int i = 0; i < LayerCount; ++i)
	{
		if (!layers[i].empty())
		{
			states.texture = (i == TextLayer ? &texture : nullptr);
			target.draw(&layers[i][0], layers[i].size(), sf::Quads, states);
		}
	}
}
