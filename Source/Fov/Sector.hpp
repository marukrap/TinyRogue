#pragma once

#include <SFML/System/Vector2.hpp>

class Sector
{
public:
	Sector(float alphaRadian = 0.f, float betaRadian = 0.f);

	void clear();
	void copy(const Sector& a);

	void setFullCircle();
	void setCone(sf::Vector2f dir, float angle, bool normalize = false);
	void setFromCoords(float cx, float cy, float ax, float ay, float bx, float by, bool normalize = false);
	void setIntersection(const Sector& a, const Sector& b);
	void setUnion(const Sector& a, const Sector& b);

private:
	float beta() const;

public:
	float alpha;
	float theta;
};
