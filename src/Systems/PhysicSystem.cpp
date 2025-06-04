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

void resolveCollision(RigidBodyComponent &a, RigidBodyComponent &b, Vector3 contactPoint, Vector3 contactNormal,
					  float restitution)
{
	Vector3 rA = Vector3Subtract(contactPoint, a.center);
	Vector3 rB = Vector3Subtract(contactPoint, b.center);

	// Velocity at contact points
	Vector3 vA = Vector3Add(a.velocity, Vector3CrossProduct(a.angularVelocity, rA));
	Vector3 vB = Vector3Add(b.velocity, Vector3CrossProduct(b.angularVelocity, rB));
	Vector3 relativeVelocity = Vector3Subtract(vA, vB);

	float contactVel = Vector3DotProduct(relativeVelocity, contactNormal);

	// Skip if separating
	if (contactVel > 0.0f)
		return;

	// Compute impulse scalar
	Vector3 rAxn = Vector3CrossProduct(rA, contactNormal);
	Vector3 rBxn = Vector3CrossProduct(rB, contactNormal);

	Vector3 IArAxn = Vector3Transform(rAxn, a.getWorldInverseInertiaTensor());
	Vector3 IBrBxn = Vector3Transform(rBxn, b.getWorldInverseInertiaTensor());

	float denom = a.invMass + b.invMass + Vector3DotProduct(contactNormal, Vector3CrossProduct(IArAxn, rA)) +
				  Vector3DotProduct(contactNormal, Vector3CrossProduct(IBrBxn, rB));

	float j = -(1.0f + restitution) * contactVel / denom;

	Vector3 impulse = Vector3Scale(contactNormal, j);

	// Apply linear impulse
	a.velocity = Vector3Add(a.velocity, Vector3Scale(impulse, a.invMass));
	b.velocity = Vector3Subtract(b.velocity, Vector3Scale(impulse, b.invMass));

	// Apply angular impulse
	if (a.invMass > 0.0f)
	{
		Matrix IinvA = a.getWorldInverseInertiaTensor();
		Vector3 angImpulseA = Vector3Transform(Vector3CrossProduct(rA, impulse), IinvA);
		a.angularVelocity = Vector3Add(a.angularVelocity, angImpulseA);
	}
	if (b.invMass > 0.0f)
	{
		Matrix IinvB = b.getWorldInverseInertiaTensor();
		Vector3 angImpulseB = Vector3Transform(Vector3CrossProduct(rB, impulse), IinvB);
		b.angularVelocity = Vector3Subtract(b.angularVelocity, angImpulseB);
	}
}

void resolveCollisionWithGround(RigidBodyComponent &body, Vector3 contactNormal, Vector3 contactPoint,
								float restitution)
{
	Vector3 r = Vector3Subtract(contactPoint, body.center);

	Vector3 velocityAtContact = Vector3Add(body.velocity, Vector3CrossProduct(body.angularVelocity, r));
	float contactVel = Vector3DotProduct(velocityAtContact, contactNormal);

	if (contactVel > 0.0f)
		return; // moving away

	// 👇 Clamp restitution at low speeds
	if (fabsf(contactVel) < 0.2f)
		restitution = 0.0f;

	float invMass = (body.mass > 0.0f) ? 1.0f / body.mass : 0.0f;

	Vector3 rCrossN = Vector3CrossProduct(r, contactNormal);
	Vector3 inertiaTerm = Vector3Transform(rCrossN, body.getWorldInverseInertiaTensor());
	float angular = Vector3DotProduct(contactNormal, Vector3CrossProduct(inertiaTerm, r));

	float denom = invMass + angular;
	float j = -(1.0f + restitution) * contactVel / denom;

	Vector3 impulse = Vector3Scale(contactNormal, j);

	body.velocity = Vector3Add(body.velocity, Vector3Scale(impulse, invMass));

	if (body.invMass > 0.0f)
	{
		Matrix Iinv = body.getWorldInverseInertiaTensor();
		Vector3 angImpulse = Vector3Transform(Vector3CrossProduct(r, impulse), Iinv);
		body.angularVelocity = Vector3Add(body.angularVelocity, angImpulse);
	}
	// Tangential velocity (relative to ground)
	// Vector3 tangentVel = Vector3Subtract(velocityAtContact, Vector3Scale(contactNormal, contactVel));
	// if (Vector3LengthSqr(tangentVel) > 0.001f)
	// {
	// 	Vector3 frictionDir = Vector3Normalize(tangentVel);
	// 	float frictionCoeff = 0.5f; // tune this
	// 	Vector3 frictionImpulse = Vector3Scale(frictionDir, -j * frictionCoeff);
	//
	// 	body.velocity = Vector3Add(body.velocity, Vector3Scale(frictionImpulse, invMass));
	// 	Vector3 angFriction =
	// 		Vector3Transform(Vector3CrossProduct(r, frictionImpulse), body.getWorldInverseInertiaTensor());
	// 	body.angularVelocity = Vector3Add(body.angularVelocity, angFriction);
	// }
}

void positionalCorrectionGround(RigidBodyComponent &body, Vector3 normal, float penetration)
{
	const float percent = 0.2f; // reduce if still jittering
	const float slop = 0.01f;	// tolerance before correcting

	float correctionDepth = fmax(penetration - slop, 0.0f);
	Vector3 correction = Vector3Scale(normal, correctionDepth * percent);
	body.center = Vector3Add(body.center, correction);

	// Optional: zero out tiny velocities to stop micro-jitter
	if (Vector3LengthSqr(body.velocity) < 0.001f)
		body.velocity = {0, 0, 0};
	if (Vector3LengthSqr(body.angularVelocity) < 0.001f)
		body.angularVelocity = {0, 0, 0};
}

void positionalCorrection(RigidBodyComponent &A, RigidBodyComponent &B, Vector3 contactNormal, float penetration)
{
	const float percent = 0.2f; // correction percentage
	const float slop = 0.01f;	// penetration allowance

	if (penetration < slop)
		return;
	float correctionMagnitude = std::max(penetration - slop, 0.0f) / (A.invMass + B.invMass) * percent;
	Vector3 correction = Vector3Scale(contactNormal, correctionMagnitude);

	A.center = Vector3Add(A.center, Vector3Scale(correction, A.invMass));
	B.center = Vector3Subtract(B.center, Vector3Scale(correction, B.invMass));
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

	Vector3 groundNormal = {0, 1, 0}; // pointing up
	float groundHeight = 0.0f;

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
				resolveCollision(box1, box2, contactPoint, contactNormal, 0.6);
				positionalCorrection(box1, box2, contactNormal, penetration);
			}
		}
	}

	for (auto entity : view)
	{
		RigidBodyComponent &body = registry.get<RigidBodyComponent>(entity);
		float maxPenetration = 0.0f;
		Vector3 deepestPoint = {};
		bool hasContact = false;

		for (int i = 0; i < 8; i++)
		{
			Vector3 vertex = getOBBVertex(body.obb, i);
			float penetration = groundHeight - vertex.y;
			if (penetration > 0.001f) // only consider if actually penetrating
			{
				hasContact = true;
				if (penetration > maxPenetration)
				{
					maxPenetration = penetration;
					deepestPoint = vertex;
				}
			}
		}

		if (hasContact)
		{
			resolveCollisionWithGround(body, groundNormal, deepestPoint, 0.0f);
			positionalCorrectionGround(body, groundNormal, maxPenetration);
		}
	}
}
