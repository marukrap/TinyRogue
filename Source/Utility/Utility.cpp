#include "Utility.hpp"

#include <random>
#include <cwctype> // towupper

namespace
{
	std::random_device rd;
	std::mt19937 mt(rd());
}

int randomInt(int exclusiveMax)
{
	return std::uniform_int_distribution<>(0, exclusiveMax - 1)(mt);
}

int randomInt(int min, int inclusiveMax)
{
	return min + std::uniform_int_distribution<>(0, inclusiveMax - min)(mt);
}

int rollDice(int number, int sides)
{
	int result = 0;

	for (int i = 0; i < number; ++i)
		result += randomInt(1, sides);

	return result;
}

int spread(int number)
{
	return randomInt(number * 4 / 5, number * 6 / 5);
}

int length(const sf::Vector2i& vector)
{
	return static_cast<int>(std::sqrt(vector.x * vector.x + vector.y * vector.y));
}

int lengthSquared(const sf::Vector2i& vector)
{
	return vector.x * vector.x + vector.y * vector.y;
}

bool contains(const std::wstring& str, wchar_t ch)
{
	return str.find(ch) != std::wstring::npos;
}

bool startsWith(const std::wstring& str, const std::wstring& start)
{
	if (start.size() > str.size())
		return false;

	return str.compare(0, start.size(), start) == 0;
}

bool isVowel(const std::wstring& str)
{
	return contains(L"aeiou", str[0]) && !startsWith(str, L"uni") && !startsWith(str, L"eu");
}

std::wstring capitalize(const std::wstring& str)
{
	std::wstring result = str;
	
	result[0] = std::towupper(str[0]);

	return result;
}
