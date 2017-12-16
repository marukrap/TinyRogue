#pragma once

#include <SFML/System/Vector2.hpp>

#include <string>

int randomInt(int exclusiveMax); // [0, max)
int randomInt(int min, int inclusiveMax); // [min, max]
int rollDice(int number, int sides); // roll S sided dice N times

/* misc.c */
// spread: Give a spread around a given number (+/- 20%)
int spread(int number);

int length(const sf::Vector2i& vector);
int lengthSquared(const sf::Vector2i& vector);

// String helper
bool contains(const std::wstring& str, wchar_t ch);
bool startsWith(const std::wstring& str, const std::wstring& start);
bool isVowel(const std::wstring& str);

std::wstring capitalize(const std::wstring& str);

template <typename T>
int sign(T value)
{
	return (value > static_cast<T>(0)) - (value < static_cast<T>(0));
}
