#pragma once

#include <SFML/Graphics/Text.hpp>

#include <deque>

class Actor;
class Item;

struct Word
{
	Word(const std::wstring& str);
	Word(const std::wstring& str, const sf::Color& color);

	Word(const Actor& actor);
	Word(const Item& item);

	std::wstring str;
	sf::Color color = sf::Color::White;
};

class MessageLog : public sf::Drawable, public sf::Transformable
{
public:
	MessageLog(int width, int height, const sf::Font& font, int fontSize, const sf::Vector2f& tileSize);

	void message(const std::wstring& str, const sf::Color& color = sf::Color::White);
	void message(const std::wstring& str, const std::vector<Word>& words, const sf::Color& color = sf::Color::White);

	void popBack();

	// Variadic templates
	// TODO: C++17 fold expressions
	/*
	template <typename T, typename... Args>
	void print(const std::wstring& str, const sf::Color& color, const T& t, const Args&... args)
	{
		newLine();

		std::wistringstream iss(str);
		std::wstring substr;
		
		std::vector<Word> vector;
		vector.emplace_back(t);
		int dummy[] = { (vector.emplace_back(args), 0) ... };

		while (std::getline(iss, substr, L'{'))
		{
			if (!substr.empty())
				*this << color << substr;

			if (std::getline(iss, substr, L'}'))
			{
				std::size_t i = std::stoi(substr);
				*this << vector[i].color << vector[i].str;
			}
		}
	}
	*/

private:
	MessageLog& operator<<(const sf::Color& color);
	MessageLog& operator<<(const std::wstring& str);
	// MessageLog& operator<<(int value);

	void newLine();
	void wrapText(sf::Text& text);

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	const sf::Font& font;
	const int fontSize;

	sf::Vector2i size;
	sf::Vector2f tileSize;
	sf::Vector2f origin;

	sf::Color color = sf::Color::White;
	std::deque<std::vector<sf::Text>> messages;

	// REMOVE:
	bool upper = true;
};
