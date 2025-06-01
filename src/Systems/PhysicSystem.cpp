#include "BVH.hpp"
#include "OBB.hpp"
#include "Shape.hpp"
#include "raylib.h"
//
#include "PhysicSystem.hpp"
#include "RigidBody.hpp"
#include "entt.hpp"
#include "raymath.h"
#include <cfloat>
#include <iostream>
#include <numeric>
#include <ostream>
#include <vector>

Vector3 findDeepestPoint(const OBB &box, Vector3 normal)
{
	Vector3 deepest = box.position;
	float minProj = FLT_MAX;

	for (int i = 0; i < 8; i++)
	{
		Vector3 vertex = getOBBVertex(box, i); // returns world-space corner
		float proj = Vector3DotProduct(vertex, normal);
		if (proj < minProj)
		{
			minProj = proj;
			deepest = vertex;
		}
	}
	return deepest;
}

void resolveCollision(RigidBodyComponent &A, RigidBodyComponent &B, Vector3 contactPoint, Vector3 contactNormal,
					  float restitution)
{
	Vector3 rA = Vector3Subtract(contactPoint, A.center);
	Vector3 rB = Vector3Subtract(contactPoint, B.center);

	// Velocity at contact points
	Vector3 vA = Vector3Add(A.velocity, Vector3CrossProduct(A.angularVelocity, rA));
	Vector3 vB = Vector3Add(B.velocity, Vector3CrossProduct(B.angularVelocity, rB));
	Vector3 relativeVelocity = Vector3Subtract(vA, vB);

	float contactVel = Vector3DotProduct(relativeVelocity, contactNormal);

	// Skip if separating
	if (contactVel > 0.0f)
		return;

	// Compute impulse scalar
	Vector3 rAxn = Vector3CrossProduct(rA, contactNormal);
	Vector3 rBxn = Vector3CrossProduct(rB, contactNormal);

	Vector3 IArAxn = Vector3Transform(rAxn, A.inverseInertiaTensor);
	Vector3 IBrBxn = Vector3Transform(rBxn, B.inverseInertiaTensor);

	float denom = A.invMass + B.invMass + Vector3DotProduct(contactNormal, Vector3CrossProduct(IArAxn, rA)) +
				  Vector3DotProduct(contactNormal, Vector3CrossProduct(IBrBxn, rB));

	float j = -(1.0f + restitution) * contactVel / denom;

	Vector3 impulse = Vector3Scale(contactNormal, j);

	// Apply linear impulse
	A.velocity = Vector3Add(A.velocity, Vector3Scale(impulse, 1.0f / A.mass));
	B.velocity = Vector3Subtract(B.velocity, Vector3Scale(impulse, 1.0f / B.mass));

	// Apply angular impulse
	if (A.invMass > 0.0f)
		A.angularVelocity =
			Vector3Add(A.angularVelocity, Vector3Transform(Vector3CrossProduct(rA, impulse), A.inverseInertiaTensor));
	if (B.invMass > 0.0f)
		B.angularVelocity = Vector3Subtract(B.angularVelocity,
											Vector3Transform(Vector3CrossProduct(rB, impulse), B.inverseInertiaTensor));
}

void positionalCorrection(RigidBodyComponent &A, RigidBodyComponent &B, Vector3 contactNormal, float penetration)
{
	const float percent = 0.2f; // correction percentage
	const float slop = 0.01f;	// penetration allowance

	float correctionMagnitude = std::max(penetration - slop, 0.0f) / (1.0f / A.mass + 1.0f / B.mass) * percent;
	Vector3 correction = Vector3Scale(contactNormal, correctionMagnitude);

	A.center = Vector3Add(A.center, Vector3Scale(correction, 1.0f / A.mass));
	B.center = Vector3Subtract(B.center, Vector3Scale(correction, 1.0f / B.mass));
}

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
			RigidBodyComponent &box1 = registry.get<RigidBodyComponent>(entities[i]);
			RigidBodyComponent &box2 = registry.get<RigidBodyComponent>(entities[idx]);
			Vector3 contactNormal;
			float penetration;
			if (checkOBBCollision(box1.obb, box2.obb, contactNormal, penetration))
			{
				if (penetration < 0.001f)
					continue;
				Vector3 contactPoint = findDeepestPoint(box2.obb, Vector3Negate(contactNormal));
				resolveCollision(box1, box2, contactPoint, contactNormal, 1);
				positionalCorrection(box1, box2, contactNormal, penetration);
			}
		}
	}
}
