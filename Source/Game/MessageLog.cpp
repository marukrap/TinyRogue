#include "MessageLog.hpp"
#include "../Game/Console.hpp"
#include "../World/Actor.hpp"
#include "../World/Item.hpp"
#include "../Utility/Utility.hpp"

#include <SFML/Graphics/RenderTarget.hpp>

#include <sstream>
#include <iostream>
#include <cwctype> // iswalpha

namespace
{
	bool isEnglish(wchar_t ch)
	{
		return (ch >= L'a' && ch <= L'z') || (ch >= L'A' && ch <= L'Z');
	}

	bool isKorean(wchar_t ch)
	{
		return ch >= 0xac00 && ch <= 0xd7a3; // L'°¡' - L'ÆR'
	}

	// ÇÑ±Û ¹ÞÄ§ °Ë»ç
	bool hasFinalConsonant(wchar_t ch)
	{
		return (ch - 0xac00) % 28 != 0;
	}
}

Word::Word(const std::wstring& str)
	: str(str)
{
}

Word::Word(const std::wstring& str, const sf::Color& color)
	: str(str)
	, color(color)
{
}

Word::Word(const Actor& actor)
	: str(actor.getTheName())
	, color(actor.getColor())
{
}

Word::Word(const Item& item)
	: str(item.getAName())
	, color(item.getColor())
{
}

MessageLog::MessageLog(int width, int height, const sf::Font& font, int fontSize, const sf::Vector2f& tileSize)
	: font(font)
	, fontSize(fontSize)
	, size(width, height)
	, tileSize(tileSize)
{
	sf::FloatRect bounds = sf::Text(L'A', font, fontSize).getLocalBounds();
	float topSpace = bounds.top;
	float bottomSpace = tileSize.y - (bounds.top + bounds.height);
	origin.y = std::floor((topSpace - bottomSpace) / 2.f);
	std::cout << "MessageLog Origin: " << origin.x << ", " << origin.y << '\n';
}

void MessageLog::message(const std::wstring& str, const sf::Color& color)
{
	newLine();
	upper = true;

	*this << color << str;
}

void MessageLog::message(const std::wstring& str, const std::vector<Word>& words, const sf::Color& color)
{
	newLine();
	upper = true;

	std::wistringstream iss(str);
	std::wstring substr;

	while (std::getline(iss, substr, L'{'))
	{
		if (!substr.empty())
			*this << color << substr;

		if (std::getline(iss, substr, L'}'))
		{
			std::size_t i = std::stoi(substr);
			*this << words[i].color << words[i].str;
		}
	}
}

void MessageLog::popBack()
{
	messages.pop_back();
}

MessageLog& MessageLog::operator<<(const sf::Color& color)
{
	this->color = color;

	return *this;
}

MessageLog& MessageLog::operator<<(const std::wstring& str)
{
	std::wstring curStr = str;
	std::size_t leftBracket = str.find(L'[');

	if (leftBracket != std::wstring::npos)
	{
		std::size_t rightBracket = str.find(L']');

		if (isEnglish(str[leftBracket + 1]))
		{
			// TODO: Handle English articles(a/an/the) and plural forms(s/es)
		}

		else if (isKorean(str[leftBracket + 1]))
		{
			std::size_t verticalBar = str.find(L'|');

			wchar_t ch; // ÀÌÀü ¹®ÀåÀÇ ¸¶Áö¸· ±ÛÀÚ
			std::wstring pp; // Àº/´Â, À»/¸¦, ÀÌ/°¡, ...

			if (leftBracket == 0)
				ch = static_cast<std::wstring>(messages.back().back().getString()).back();
			else
				ch = str[leftBracket - 1];

			if (hasFinalConsonant(ch))
				pp = str.substr(leftBracket + 1, verticalBar - (leftBracket + 1));
			else
				pp = str.substr(verticalBar + 1, rightBracket - (verticalBar + 1));

			curStr = str.substr(0, leftBracket) + pp + str.substr(rightBracket + 1);
		}
	}

	// if (messages.back().empty())
	if (upper)
	{
		curStr = capitalize(curStr);
		upper = false;
	}

	sf::Text text(curStr, font, fontSize);
	text.setFillColor(color);

	if (messages.back().empty())
	{
		text.setPosition(0.f, (messages.size() - 1) * tileSize.y);
		text.setOrigin(origin);
	}

	else
	{
		const sf::Text& prevText = messages.back().back();
		const std::wstring& prevStr = prevText.getString();

		sf::Vector2f pos = prevText.findCharacterPos(prevStr.size());

		if (!prevStr.empty() && !curStr.empty())
		{
			sf::Uint32 prevChar = prevStr.back();
			sf::Uint32 curChar = curStr.front();

			// Apply the kerning offset
			pos.x += font.getKerning(prevChar, curChar, fontSize);
		}

		text.setPosition(pos);
	}

	sf::FloatRect bounds = text.getLocalBounds();

	if (text.getPosition().x + bounds.left + bounds.width > size.x * tileSize.x)
		wrapText(text);
	else
		messages.back().emplace_back(std::move(text));

	color = sf::Color::White;

	return *this;
}

void MessageLog::newLine()
{
	messages.emplace_back();

	if (static_cast<int>(messages.size()) > size.y)
	{
		messages.pop_front();

		for (auto& texts : messages)
			for (auto& text : texts)
				text.move(0.f, -tileSize.y);
	}
}

void MessageLog::wrapText(sf::Text& text)
{
	std::wstring curStr = text.getString();
	std::wstring nextStr;

	sf::FloatRect bounds;

	do
	{
		std::size_t lastSpace = curStr.find_last_of(L' ');

		if (lastSpace != std::wstring::npos)
		{
			nextStr = curStr.substr(lastSpace) + nextStr;
			curStr = curStr.substr(0, lastSpace);
			text.setString(curStr);
		}

		else
		{
			/*
			if (!messages.back().empty())
			{
				nextStr = curStr + nextStr;
				break;
			}

			else
			*/
			{
				nextStr = curStr.back() + nextStr;
				curStr.pop_back();
				text.setString(curStr);
			}
		}

		bounds = text.getLocalBounds();

	} while (text.getPosition().x + bounds.left + bounds.width > size.x * tileSize.x);

	if (!curStr.empty())
		messages.back().emplace_back(std::move(text));

	if (!nextStr.empty())
	{
		if (nextStr[0] == L' ')
			nextStr.erase(0, 1);

		newLine();

		*this << nextStr;
	}
}

void MessageLog::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();

	for (const auto& texts : messages)
		for (const auto& text : texts)
			target.draw(text, states);
}
