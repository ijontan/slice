#include "BVH.hpp"
#include "FixedJoint.hpp"
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
		float proj = Vector3DotProduct(Vector3Subtract(vertex, box.position), normal);
		if (proj < minProj)
		{
			minProj = proj;
			deepest = vertex;
		}
	}
	return deepest;
}

Vector3 QuaternionLog(Quaternion q)
{
	float sinTheta = sqrtf(q.x * q.x + q.y * q.y + q.z * q.z);
	float theta = acosf(Clamp(q.w, -1.0f, 1.0f)); // Clamp for safety

	if (sinTheta < 1e-6f || theta < 1e-6f)
		return {0.0f, 0.0f, 0.0f};

	float scale = theta / sinTheta;
	return {q.x * scale, q.y * scale, q.z * scale};
}
Quaternion QuaternionExp(Vector3 v)
{
	float theta = Vector3Length(v);
	if (theta < 1e-6f)
		return {0.0f, 0.0f, 0.0f, 1.0f}; // No rotation

	float sinTheta = sinf(theta);
	float cosTheta = cosf(theta);
	float scale = sinTheta / theta;

	return {v.x * scale, v.y * scale, v.z * scale, cosTheta};
}

void solveJoints(entt::registry &registry)
{
	auto view = registry.view<FixedJoint>();

	for (auto entity : view)
	{
		auto &joint = registry.get<FixedJoint>(entity);
		auto &a = registry.get<RigidBodyComponent>(joint.a);
		auto &b = registry.get<RigidBodyComponent>(joint.b);

		if (a.invMass == 0.0f && b.invMass == 0.0f)
			continue;

		//==================================================
		// 1. Angular Constraint - Match Relative Orientation
		{
			Quaternion qRel = QuaternionMultiply(QuaternionInvert(a.orientation), b.orientation);
			Quaternion qError = QuaternionMultiply(qRel, QuaternionInvert(joint.initialRotationOffset));

			if (qError.w < 0)
			{
				qError.x = -qError.x;
				qError.y = -qError.y;
				qError.z = -qError.z;
				qError.w = -qError.w;
			}

			Vector3 angularError = QuaternionLog(qError);

			// Clamp correction
			float angle = Vector3Length(angularError);
			if (angle > 0.6f)
				angularError = Vector3Scale(Vector3Normalize(angularError), 0.6f);

			// Scale down for stability
			angularError = Vector3Scale(angularError, 0.2f);

			// Apply half to each body (directly, not through inertia)
			if (a.invMass > 0.0f && b.invMass > 0.0f)
			{
				Quaternion delta = QuaternionExp(Vector3Scale(angularError, 0.5f));
				a.orientation = QuaternionNormalize(QuaternionMultiply(delta, a.orientation));
				b.orientation = QuaternionNormalize(QuaternionMultiply(b.orientation, QuaternionInvert(delta)));
			}
			else if (a.invMass > 0.0f)
			{
				Quaternion delta = QuaternionExp(Vector3Negate(angularError));
				a.orientation = QuaternionNormalize(QuaternionMultiply(delta, a.orientation));
			}
			else if (b.invMass > 0.0f)
			{
				Quaternion delta = QuaternionExp(angularError);
				b.orientation = QuaternionNormalize(QuaternionMultiply(b.orientation, delta));
			}
		} //==================================================

		//===========================================
		// 2. Linear Constraint - Match Anchor Points
		//===========================================
		{
			Vector3 worldAnchorA = Vector3Add(a.center, Vector3RotateByQuaternion(joint.localAnchorA, a.orientation));
			Vector3 worldAnchorB = Vector3Add(b.center, Vector3RotateByQuaternion(joint.localAnchorB, b.orientation));
			Vector3 error = Vector3Subtract(worldAnchorB, worldAnchorA);

			float errorLength = Vector3Length(error);
			if (errorLength < 1e-5f)
				continue;
			Vector3 dir = Vector3Scale(error, 1.0f / errorLength);

			// Relative anchors
			Vector3 rA = Vector3Subtract(worldAnchorA, a.center);
			Vector3 rB = Vector3Subtract(worldAnchorB, b.center);

			Vector3 raCrossN = Vector3CrossProduct(rA, dir);
			Vector3 rbCrossN = Vector3CrossProduct(rB, dir);

			Vector3 invInertiaRa = Vector3Transform(raCrossN, a.getWorldInverseInertiaTensor());
			Vector3 invInertiaRb = Vector3Transform(rbCrossN, b.getWorldInverseInertiaTensor());

			float angularFactor = Vector3DotProduct(Vector3CrossProduct(invInertiaRa, rA), dir) +
								  Vector3DotProduct(Vector3CrossProduct(invInertiaRb, rB), dir);

			float k = a.invMass + b.invMass + angularFactor;

			if (k > 0.0f)
			{
				float lambda = -errorLength / k;
				Vector3 impulse = Vector3Scale(dir, lambda * 0.1);
				if (Vector3Length(impulse) > 0.3f)
					impulse = Vector3Scale(Vector3Normalize(impulse), 0.3f);

				if (a.invMass > 0.0f)
				{
					a.center = Vector3Subtract(a.center, Vector3Scale(impulse, a.invMass));
					Vector3 dOmegaA =
						Vector3Transform(Vector3CrossProduct(rA, impulse), a.getWorldInverseInertiaTensor());
					a.orientation =
						QuaternionNormalize(QuaternionMultiply(a.orientation, QuaternionExp(Vector3Negate(dOmegaA))));
				}
				if (b.invMass > 0.0f)
				{
					b.center = Vector3Add(b.center, Vector3Scale(impulse, b.invMass));
					Vector3 dOmegaB =
						Vector3Transform(Vector3CrossProduct(rB, impulse), b.getWorldInverseInertiaTensor());
					b.orientation = QuaternionNormalize(QuaternionMultiply(b.orientation, QuaternionExp(dOmegaB)));
				}
			}
		}
	}
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

	if (denom == 0.0f)
		return;

	float j = -(1.0f + restitution) * contactVel / denom;

	Vector3 impulse = Vector3Scale(contactNormal, j);

	// Apply impulse
	if (a.invMass > 0.0f)
	{
		a.velocity = Vector3Add(a.velocity, Vector3Scale(impulse, a.invMass));
		Vector3 angImpulseA = Vector3Transform(Vector3CrossProduct(rA, impulse), a.getWorldInverseInertiaTensor());
		a.angularVelocity = Vector3Add(a.angularVelocity, angImpulseA);
	}
	if (b.invMass > 0.0f)
	{
		b.velocity = Vector3Subtract(b.velocity, Vector3Scale(impulse, b.invMass));
		Vector3 angImpulseB = Vector3Transform(Vector3CrossProduct(rB, impulse), b.getWorldInverseInertiaTensor());
		b.angularVelocity = Vector3Subtract(b.angularVelocity, angImpulseB);
	}
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

	for (int i = 0; i < 100; ++i)
		solveJoints(registry);

	for (auto entity : view)
	{
		RigidBodyComponent &rigidBody = registry.get<RigidBodyComponent>(entity);
		BoxComponent &box = registry.get<BoxComponent>(entity);

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

		RigidBodyComponent &box1 = registry.get<RigidBodyComponent>(entities[i]);
		for (auto idx : result)
		{
			if (static_cast<unsigned long>(idx) <= i)
				continue;
			RigidBodyComponent &box2 = registry.get<RigidBodyComponent>(entities[idx]);
			Vector3 contactNormal;
			float penetration;
			if (!box1.shouldCollideWith(box2))
				continue;
			if (checkOBBCollision(box1.obb, box2.obb, contactNormal, penetration))
			{
				if (penetration < 0.01f)
					continue;
				Vector3 contactPoint = findDeepestPoint(box2.obb, contactNormal);
				resolveCollision(box1, box2, contactPoint, contactNormal, 0.6);
				positionalCorrection(box1, box2, contactNormal, penetration);
			}
		}
		box1.intergrate(deltaTime);
	}
}
