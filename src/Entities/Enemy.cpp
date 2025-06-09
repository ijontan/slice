
#include "Block.hpp"
#include "EnemyState.hpp"
#include "Entity.hpp"
#include "RigidBody.hpp"
#include "Scene.hpp"
#include "math.hpp"
#include "raylib.h"
#include "raymath.h"
#include <iostream>

void createEnemy(Scene &scene)
{
	Entity enemy = scene.createEntity();
	EnemyState state = {};
	state.size = {3, 3, 3};
	state.regenDuration = 0.2;

	int size = state.getSize();
	state.parts.reserve(size);

	int coreIndex = randomInt(0, size-1) ;
	state.cores.push_back(state.getCoordFromIndex(coreIndex));

	BlockFactory blockFactory(scene);
	blockFactory.setPosition({randomFloat(-30, 30), 10, randomFloat(-30, 30)});
	Vector3 halfSize = {1.0f, 1.0f, 1.0f};
	blockFactory.setDimension(Vector3Scale(halfSize, 2.0f));
	blockFactory.setVelocity({0.0f, 0.0f, 0.0f});
	blockFactory.setAngularVelocity({0.0f, 0.0f, 0.0f});
	blockFactory.setCategory(CollisionMask::ENEMY);
	blockFactory.setCollisionMask(CollisionMask::FREE | CollisionMask::PLAYER);
	blockFactory.setOrientation(QuaternionIdentity());

	for (int i = 0; i < size; i++)
	{
		EnemyPart part = {};
		part.halfSize = halfSize;
		if (i == coreIndex)
		{
			part.type = CORE;
			part.isAttached = true;
			blockFactory.setColor(YELLOW);
			part.entity = blockFactory.generateBlock();
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
