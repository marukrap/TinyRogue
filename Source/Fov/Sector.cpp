#include "Sector.hpp"

#include <cmath> // atan2
#include <algorithm> // min, max

namespace Angle
{
	const float PI = 3.14159265358979323846f;
	const float TwoPI = 2 * PI;

	// Normalizes angle to be between 0 and 2PI
	float normalizeRad2(float angle)
	{
		while (angle < 0.f)
			angle += TwoPI;
		while (angle > TwoPI)
			angle -= TwoPI;

		return angle;
	}
}

Sector::Sector(float alphaRadian, float betaRadian)
	: alpha(alphaRadian)
	, theta(betaRadian - alphaRadian)
{
}

void Sector::clear()
{
	alpha = theta = 0.f;
}

void Sector::copy(const Sector& a)
{
	alpha = a.alpha;
	theta = a.theta;
}

void Sector::setFullCircle()
{
	alpha = 0.f;
	theta = Angle::TwoPI;
}

void Sector::setCone(sf::Vector2f dir, float angle, bool normalize)
{
	alpha = std::atan2(dir.y, dir.x) - 0.5f * angle;
	theta = angle;

	if (normalize)
		alpha = Angle::normalizeRad2(alpha);
}

void Sector::setFromCoords(float cx, float cy, float ax, float ay, float bx, float by, bool normalize)
{
	alpha = std::atan2(ay - cy, ax - cx);
	float newBeta = std::atan2(by - cy, bx - cx);

	if (normalize)
	{
		alpha = Angle::normalizeRad2(alpha);
		newBeta = Angle::normalizeRad2(newBeta);
	}

	if (alpha >= newBeta)
		clear();
	else
		theta = newBeta - alpha;
}

void Sector::setIntersection(const Sector& a, const Sector& b)
{
	if (a.theta == 0.f || b.theta == 0.f)
		clear();

	else
	{
		alpha = std::max(a.alpha, b.alpha);
		float newBeta = std::min(a.beta(), b.beta());

		if (alpha >= newBeta)
			clear();
		else
			theta = newBeta - alpha;
	}
}

void Sector::setUnion(const Sector& a, const Sector& b)
{
	if (a.theta == 0.f)
		copy(b);

	else if (b.theta == 0.f)
		copy(a);

	else
	{
		alpha = std::min(a.alpha, b.alpha);
		float newBeta = std::max(a.beta(), b.beta());

		if (alpha >= newBeta)
			clear();
		else
			theta = newBeta - alpha;
	}
}

float Sector::beta() const
{
	return alpha + theta;
}
