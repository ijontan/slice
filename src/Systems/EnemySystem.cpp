#include "EnemySystem.hpp"
#include "Block.hpp"
#include "EnemyState.hpp"
#include "FixedJoint.hpp"
#include "RigidBody.hpp"
#include "Scene.hpp"
#include "Shape.hpp"
#include "raylib.h"
#include "raymath.h"
#include <cmath>
#include <iostream>

void generateBody(EnemyState &state, Scene &scene, EnemyPart &partA, RigidBodyComponent &bodyA, IVector3 coord,
				  IVector3 next)
{
	EnemyPart &partB = state.getPartRef({
		coord.x + next.x,
		coord.y + next.y,
		coord.z + next.z,
	});
	if (partB.isAttached)
		return;
	FixedJoint joint = {};
	Vector3 direction = Vector3RotateByQuaternion({(float)next.x, (float)next.y, (float)next.z}, bodyA.orientation);

	float scale = fabsf((float)next.x) * (partA.halfSize.x + partB.halfSize.x) +
				  fabsf((float)next.y) * (partA.halfSize.y + partB.halfSize.y) +
				  fabsf((float)next.z) * (partA.halfSize.z + partB.halfSize.z);

	Vector3 position = Vector3Add(bodyA.center, Vector3Scale(direction, scale));
	partB.entity = setupBlock(scene, position, Vector3Scale(partB.halfSize, 2.0f), {0, 0, 0}, {0, 0, 0},
							  CollisionMask::ENEMY, CollisionMask::FREE | CollisionMask::PLAYER, bodyA.orientation);

	joint.localAnchorA = {partA.halfSize.x * (float)next.x, partA.halfSize.y * (float)next.y,
						  partA.halfSize.z * (float)next.z};
	joint.localAnchorB = {-partB.halfSize.x * (float)next.x, -partB.halfSize.y * (float)next.y,
						  -partB.halfSize.z * (float)next.z};
	joint.a = partA.entity;
	joint.b = partB.entity;
	joint.initialRotationOffset =
		QuaternionNormalize(QuaternionMultiply(QuaternionInvert(bodyA.orientation), bodyA.orientation));
	partB.entity.addComponent<FixedJoint>(joint);
	partB.isAttached = true;
	partB.isNew = true;
}

void enemyRegenBodySystem(entt::registry &registry, Scene &scene)
{
	auto view = registry.view<EnemyState>();
	float deltaTime = GetFrameTime();

	for (auto entity : view)
	{
		EnemyState &state = registry.get<EnemyState>(entity);

		state.timePassedLastRegen += deltaTime;

		if (state.timePassedLastRegen < state.regenDuration)
			continue;

		state.timePassedLastRegen = 0;

		int size = state.getSize();
		for (int i = 0; i < size; ++i)
		{

			IVector3 coord = state.getCoordFromIndex(i);
			EnemyPart &partA = state.getPartRef(coord);
			if (!partA.isAttached || partA.isNew)
				continue;
			auto &bodyA = registry.get<RigidBodyComponent>(partA.entity);

			if (coord.x > 0)
				generateBody(state, scene, partA, bodyA, coord, {-1, 0, 0});
			if (coord.y > 0)
				generateBody(state, scene, partA, bodyA, coord, {0, -1, 0});
			if (coord.z > 0)
				generateBody(state, scene, partA, bodyA, coord, {0, 0, -1});
			if (coord.x < state.size.x - 1)
				generateBody(state, scene, partA, bodyA, coord, {1, 0, 0});
			if (coord.y < state.size.y - 1)
				generateBody(state, scene, partA, bodyA, coord, {0, 1, 0});
			if (coord.z < state.size.z - 1)
				generateBody(state, scene, partA, bodyA, coord, {0, 0, 1});
		}
		for (auto &part : state.parts)
		{
			part.isNew = false;
			(void)part;
		}
	}
}
