#include <algorithm> // shuffle

template <typename T>
const T& Rng::getOne(const std::vector<T>& vector)
{
	return vector[getInt(vector.size())];
}

template <typename T>
void Rng::shuffle(std::vector<T>& vector)
{
	std::shuffle(vector.begin(), vector.end(), mt);
}
