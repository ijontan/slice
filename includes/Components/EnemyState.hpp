#include "Entity.hpp"
#include "error.hpp"
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

struct IVector3
{
	int x;
	int y;
	int z;
};

struct EnemyPart
{
	EnemyPartType type;
	bool isAttached;
	bool isNew;
	Entity entity;
	Vector3 halfSize;
};

struct EnemyState
{
	IVector3 size;
	std::vector<IVector3> cores;
	std::vector<EnemyPart> parts;
	float regenDuration;
	float timePassedLastRegen;

	int getSize()
	{
		return size.x * size.y * size.z;
	}

	IVector3 getCoordFromIndex(int i)
	{
		int z = i / (size.x * size.y);
		int y = i % (size.x * size.y) / size.x;
		int x = i % (size.x * size.y) % size.x;
		return {x, y, z};
	}

	EnemyPart &getPartRef(IVector3 coord)
	{
		int i = coord.x + coord.y * size.x + coord.z * size.x * size.y;
		if (i > getSize())
			throw Error("getPartRef: ", "exceed max size.");
		return parts[i];
	}
};
