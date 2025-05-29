#include "BVH.hpp"
#include "OBB.hpp"
#include "Shape.hpp"
#include "raylib.h"
//
#include "RigidBody.hpp"
#include "entt.hpp"
#include "raymath.h"
#include <iostream>
#include <vector>

void stepPhysicSimulation(entt::registry &registry)
{
	auto deltaTime = GetFrameTime();
	auto view = registry.view<RigidBodyComponent, BoxComponent>();
	std::vector<entt::entity> entities(view.size_hint());
	std::vector<BoundingBox> boundingBoxes(view.size_hint());

	for (auto entity : view)
	{
		RigidBodyComponent &rigidBody = registry.get<RigidBodyComponent>(entity);
		BoxComponent &box = registry.get<BoxComponent>(entity);
		rigidBody.Intergrate(deltaTime);

		Matrix transform = MatrixMultiply(QuaternionToMatrix(rigidBody.orientation),
										  MatrixTranslate(rigidBody.center.x, rigidBody.center.y, rigidBody.center.z));
		box.model.transform = transform;

		rigidBody.obb.position = rigidBody.center;
		rigidBody.obb.xAxis = Vector3RotateByQuaternion({1, 0, 0}, rigidBody.orientation);
		rigidBody.obb.yAxis = Vector3RotateByQuaternion({0, 1, 0}, rigidBody.orientation);
		rigidBody.obb.zAxis = Vector3RotateByQuaternion({0, 0, 1}, rigidBody.orientation);

		entities.push_back(entity);
		boundingBoxes.push_back(GetMeshBoundingBox(box.model.meshes[0]));
	}

	// int count = 0;
	static BVHNode *head;
	if (head != nullptr)
	{
		bvhClean(head);
		head = nullptr;
	}
	head = bvhBuild(boundingBoxes, entities);

	int count = 0;
	// for (unsigned long i = 0; i < entities.size(); ++i)
	// {
	// 	std::vector<entt::entity> result;
	// 	bvhDetectCollision(head, boundingBoxes[i], result);
	//
	// 	for (auto entity : result)
	// 	{
	// 		if (entity == entities[i])
	// 			continue;
	// 		OBB &box1 = registry.get<RigidBodyComponent>(entities[i]).obb;
	// 		OBB &box2 = registry.get<RigidBodyComponent>(entity).obb;
	// 		if (checkCollision(box1, box2))
	// 			count++;
	// 	}
	// }
	std::cout << "number of collisions: " << count << std::endl;
	count = 0;
	// for (unsigned long i = 0; i < entities.size(); i++)
	// {
	// 	for (unsigned long j = i+1; j < entities.size(); j++)
	// 	{
	// 		OBB &box1 = registry.get<RigidBodyComponent>(entities[i]).obb;
	// 		OBB &box2 = registry.get<RigidBodyComponent>(entities[j]).obb;
	// 		if (Vector3Distance(box1.position, box2.position) > 3)
	// 			continue;
	// 		if (checkCollision(box1, box2))
	// 			count++;
	// 	}
	// }
	std::cout << "number of collisions" << count << std::endl;
}
