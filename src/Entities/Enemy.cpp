
#include "EnemyState.hpp"
#include "Entity.hpp"
#include "Scene.hpp"
#include "math.hpp"

void createEnemy(Scene &scene)
{
	Entity emeny = scene.createEntity();
	(void)emeny;
	EnemyState state = {};
	state.size = {3, 3, 3};

	int size = state.getSize();
	state.parts.reserve(size);

	int coreIndex = randomInt(0, size - 1);
	state.cores.push_back(state.getCoordFromIndex(coreIndex));

	for (int i = 0; i < size; i++)
	{
		EnemyPart part = {};
		if (i == coreIndex)
			part.type = CORE;
		else
			part.type = DEFAULT;
		state.parts.push_back(part);
	}
}
