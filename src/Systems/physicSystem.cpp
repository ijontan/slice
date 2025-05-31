#include "BVH.hpp"
#include "OBB.hpp"
#include "Shape.hpp"
#include "raylib.h"
//
#include "PhysicSystem.hpp"
#include "RigidBody.hpp"
#include "entt.hpp"
#include "raymath.h"
#include <iostream>
#include <numeric>
#include <ostream>
#include <vector>

// Vector3 getContactPoint(OBB &a, OBB & b){
// Vector3
// 	centerDelta = Vector3Subtract(b.position, a.position);
// }

void stepPhysicSimulation(entt::registry &registry)
{
	auto deltaTime = GetFrameTime();
	auto view = registry.view<RigidBodyComponent, BoxComponent>();
	std::vector<entt::entity> entities;
	std::vector<BoundingBox> boundingBoxes;
	entities.reserve(view.size_hint());
	boundingBoxes.reserve(view.size_hint());

	for (auto entity : view)
	{
		RigidBodyComponent &rigidBody = registry.get<RigidBodyComponent>(entity);
		BoxComponent &box = registry.get<BoxComponent>(entity);
		rigidBody.Intergrate(deltaTime);

		Matrix transform = MatrixMultiply(QuaternionToMatrix(rigidBody.orientation),
										  MatrixTranslate(rigidBody.center.x, rigidBody.center.y, rigidBody.center.z));
		box.model.transform = transform;

		rigidBody.obb.position = rigidBody.center;
		rigidBody.obb.axis[0] = Vector3RotateByQuaternion({1, 0, 0}, rigidBody.orientation);
		rigidBody.obb.axis[1] = Vector3RotateByQuaternion({0, 1, 0}, rigidBody.orientation);
		rigidBody.obb.axis[2] = Vector3RotateByQuaternion({0, 0, 1}, rigidBody.orientation);

		entities.push_back(entity);
		boundingBoxes.push_back(getModelBoundingBox(box.model));
	}

	// int count = 0;
	static BVHNode *head;
	if (head != nullptr)
	{
		bvhClean(head);
		head = nullptr;
	}
	std::vector<int> indexes(boundingBoxes.size());
	std::iota(indexes.begin(), indexes.end(), 0);
	head = bvhBuild(boundingBoxes, indexes);

	// bvhDisplay(head);

	// for (auto box : boundingBoxes)
	// 	DrawBoundingBox(box, BLACK);
	
	for (unsigned long i = 0; i < entities.size(); ++i)
	{
		std::vector<int> result;
		bvhDetectCollision(head, boundingBoxes[i], result);

		for (auto idx : result)
		{
			if (static_cast<unsigned long>(idx) == i)
				continue;
			OBB &box1 = registry.get<RigidBodyComponent>(entities[i]).obb;
			OBB &box2 = registry.get<RigidBodyComponent>(entities[idx]).obb;
			Vector3 contactNormal;
			float penetration;
			if (checkOBBCollision(box1, box2, contactNormal, penetration))
			{
			}
		}
	}
}
