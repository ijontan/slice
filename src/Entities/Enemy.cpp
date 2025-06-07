
#include "EnemyState.hpp"
#include "Entity.hpp"
#include "Scene.hpp"
#include "math.hpp"

void createEnemy(Scene &scene)
{
	Entity emeny = scene.createEntity();
	(void)emeny;
	EnemyState state = {};
	state.xlen = 3;
	state.ylen = 3;
	state.zlen = 3;

	int size = state.getSize();
	state.parts.reserve(size);
	state.blueprint.reserve(size);

	int coreIndex = randomInt(0, size - 1);
	for (int i = 0; i < size; i++)
	{
		EnemyPart part = {};

		part.entity = scene.createEntity();
		if (i == coreIndex)
			part.type = CORE;
		else
			part.type = DEFAULT;
		state.blueprint.push_back(part.type);
		state.parts.push_back(part);
	}
}
