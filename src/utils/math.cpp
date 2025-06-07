#include <cstdlib>

float randomFloat(float min = -1.0f, float max = 1.0f)
{
	return min + static_cast<float>(std::rand()) / (static_cast<float>(RAND_MAX) / (max - min));
}

int randomInt(int min = 0, int max = 10)
{
	return min + static_cast<int>(std::rand()) / (static_cast<float>(RAND_MAX) / (max - min));
}
