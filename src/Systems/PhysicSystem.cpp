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

Vector3 QuaternionLogSafe(Quaternion q)
{
	float sinTheta = sqrtf(q.x * q.x + q.y * q.y + q.z * q.z);
	if (sinTheta < 1e-6f)
		return (Vector3){0, 0, 0};

	float angle = acosf(q.w) * 2.0f;
	float scale = angle / sinTheta;
	return (Vector3){q.x * scale, q.y * scale, q.z * scale};
}

void solveJoints(entt::registry &registry)
{
	auto view = registry.view<FixedJoint>();
	float deltaTime = GetFrameTime();

	for (auto entity : view)
	{
		auto &joint = registry.get<FixedJoint>(entity);

		auto &a = registry.get<RigidBodyComponent>(joint.a);
		auto &b = registry.get<RigidBodyComponent>(joint.b);
		if (a.invMass == 0.0f && b.invMass == 0.0f)
			continue;

		// ================================================================
		// 1. Angular Constraint - Forcing bodies to have a fixed relative orientation
		// ================================================================
		{
			// The error is the difference between the current relative orientation and the target
			Quaternion currentRelativeOrientation = QuaternionMultiply(QuaternionInvert(a.orientation), b.orientation);
			Quaternion orientationError =
				QuaternionMultiply(currentRelativeOrientation, QuaternionInvert(joint.initialRotationOffset));

			if (orientationError.w < 0.0f)
			{
				orientationError.x = -orientationError.x;
				orientationError.y = -orientationError.y;
				orientationError.z = -orientationError.z;
				orientationError.w = -orientationError.w;
			}
			Quaternion target = QuaternionNormalize(
				QuaternionMultiply(QuaternionInvert(joint.initialRotationOffset), orientationError));
			if (target.w < 0.0f)
			{
				target.x = -target.x;
				target.y = -target.y;
				target.z = -target.z;
				target.w = -target.w;
			}
			Vector3 axis = QuaternionLogSafe(target);
			if (Vector3LengthSqr(axis) < 1e-6f)
				continue;
			Vector3 relativeAngularVelocity = Vector3Subtract(b.angularVelocity, a.angularVelocity);

			// Baumgarte stabilization to correct the orientation error over time
			float baumgarte = 0.05f;
			float scale = baumgarte / deltaTime;
			if (scale > 0.5)
				scale = 0.5;
			Vector3 bias = Vector3Scale(axis, scale);

			// Calculate effective angular mass (same as in the slider joint)
			Matrix worldInvInertiaA = a.getWorldInverseInertiaTensor();
			Matrix worldInvInertiaB = b.getWorldInverseInertiaTensor();
			Matrix invAngularMass = MatrixInvert(MatrixAdd(worldInvInertiaA, worldInvInertiaB));
			//
			// Calculate the impulse needed to counteract velocity and correct position
			Vector3 angularImpulse =
				Vector3Transform(Vector3Negate(Vector3Add(relativeAngularVelocity, bias)), invAngularMass);

			// Apply the angular impulse to both bodies
			if (a.invMass > 0.0f)
			{
				a.angularVelocity =
					Vector3Subtract(a.angularVelocity, Vector3Transform(angularImpulse, worldInvInertiaA));
			}
			if (b.invMass > 0.0f)
			{
				b.angularVelocity = Vector3Add(b.angularVelocity, Vector3Transform(angularImpulse, worldInvInertiaB));
			}
		}

		//================================================================
		// 2. Linear Constraint - Forcing anchor points to be at the same position
		//================================================================
		{
			// Get the current world-space positions of the anchors
			Vector3 worldAnchorA = Vector3Add(a.center, Vector3RotateByQuaternion(joint.localAnchorA, a.orientation));
			Vector3 worldAnchorB = Vector3Add(b.center, Vector3RotateByQuaternion(joint.localAnchorB, b.orientation));

			// The error is simply the vector between the two anchors. We want this to be zero.
			Vector3 errorVector = Vector3Subtract(worldAnchorB, worldAnchorA);
			float errorMagnitude = Vector3Length(errorVector);

			if (errorMagnitude > 0.001f)
			{
				Vector3 errorNormal = Vector3Normalize(errorVector);

				// This section is now identical to your original resolveCollision function.
				// We are essentially treating the separation of the anchor points as a "collision"
				// with a penetration depth of 'errorMagnitude'.
				Vector3 rA = Vector3Subtract(worldAnchorA, a.center);
				Vector3 rB = Vector3Subtract(worldAnchorB, b.center);

				Vector3 vA = Vector3Add(a.velocity, Vector3CrossProduct(a.angularVelocity, rA));
				Vector3 vB = Vector3Add(b.velocity, Vector3CrossProduct(b.angularVelocity, rB));
				Vector3 relativeVelocity = Vector3Subtract(vB, vA);
				float contactVel = Vector3DotProduct(relativeVelocity, errorNormal);

				float baumgarte = 0.4f;
				float bias = (baumgarte / deltaTime) * errorMagnitude;

				// Calculate the denominator for the impulse magnitude 'j'
				Vector3 rAxn = Vector3CrossProduct(rA, errorNormal);
				Vector3 rBxn = Vector3CrossProduct(rB, errorNormal);
				Vector3 IArAxn = Vector3Transform(rAxn, a.getWorldInverseInertiaTensor());
				Vector3 IBrBxn = Vector3Transform(rBxn, b.getWorldInverseInertiaTensor());

				float denom = a.invMass + b.invMass + Vector3DotProduct(errorNormal, Vector3CrossProduct(IArAxn, rA)) +
							  Vector3DotProduct(errorNormal, Vector3CrossProduct(IBrBxn, rB));

				if (denom > 0.0f)
				{
					// Calculate impulse magnitude 'j'
					// Restitution is 0 for a fixed joint.
					float j = -(contactVel + bias) / denom;
					Vector3 impulse = Vector3Scale(errorNormal, j);

					// Apply impulse
					if (a.invMass > 0.0f)
					{
						a.velocity = Vector3Subtract(a.velocity, Vector3Scale(impulse, a.invMass));
						a.angularVelocity =
							Vector3Subtract(a.angularVelocity, Vector3Transform(Vector3CrossProduct(rA, impulse),
																				a.getWorldInverseInertiaTensor()));
					}
					if (b.invMass > 0.0f)
					{
						b.velocity = Vector3Add(b.velocity, Vector3Scale(impulse, b.invMass));
						b.angularVelocity =
							Vector3Add(b.angularVelocity, Vector3Transform(Vector3CrossProduct(rB, impulse),
																		   b.getWorldInverseInertiaTensor()));
					}
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
			if (checkOBBCollision(box1.obb, box2.obb, contactNormal, penetration))
			{
				if (penetration < 0.01f)
					continue;
				if (registry.any_of<FixedJoint>(entities[i]))
				{
					FixedJoint &joint = registry.get<FixedJoint>(entities[i]);
					if (joint.a == entities[idx])
						continue;
				}
				Vector3 contactPoint = findDeepestPoint(box2.obb, contactNormal);
				resolveCollision(box1, box2, contactPoint, contactNormal, 0.6);
				positionalCorrection(box1, box2, contactNormal, penetration);
			}
		}
		box1.intergrate(deltaTime);
	}
}
