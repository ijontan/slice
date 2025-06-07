#include "Entity.hpp"
#include <vector>

enum EnemyType
{
	SMALL,
	MEDIUM,
	LARGE,
};

enum EnemyPartType
{
	AIR,
	CORE,
	DEFAULT,
};

struct EnemyPart
{
	EnemyPartType type;
	Entity entity;
};

struct EnemyState
{
	int xlen;
	int ylen;
	int zlen;
	std::vector<EnemyPart> parts;
	std::vector<EnemyPartType> blueprint;

	int getSize()
	{
		return xlen + ylen + zlen;
	}

	EnemyPart &getPartRef(int x, int y, int z)
	{
		return parts[x + y * xlen + z * xlen * ylen];
	}
};
