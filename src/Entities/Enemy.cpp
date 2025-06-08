
#include "Block.hpp"
#include "EnemyState.hpp"
#include "Entity.hpp"
#include "RigidBody.hpp"
#include "Scene.hpp"
#include "math.hpp"
#include "raymath.h"
#include <iostream>

void createEnemy(Scene &scene)
{
	Entity enemy = scene.createEntity();
	EnemyState state = {};
	state.size = {3, 3, 3};
	state.regenDuration = 5;

	int size = state.getSize();
	state.parts.reserve(size);

	int coreIndex = randomInt(0, size - 1);
	state.cores.push_back(state.getCoordFromIndex(coreIndex));

	for (int i = 0; i < size; i++)
	{
		EnemyPart part = {};
		part.halfSize = {1.0f, 1.0f, 1.0f};
		if (i == coreIndex)
		{
			part.type = CORE;
			part.isAttached = true;
			part.entity = setupBlock(scene, {5, 5, 0}, Vector3Scale(part.halfSize, 2.0f), {0, 0, 0}, {0, 0, 0},
									 CollisionMask::ENEMY, CollisionMask::FREE | CollisionMask::PLAYER, QuaternionIdentity());
		}
		else
		{
			part.type = DEFAULT;
			part.isAttached = false;
		}
		state.parts.push_back(part);
	}
	enemy.addComponent<EnemyState>(state);
}
