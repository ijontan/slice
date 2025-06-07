#pragma once
#include "OBB.hpp"
#include "raymath.h"

struct RigidBodyComponent
{
	// Mesh mesh;
	float mass = 10;
	float invMass = 0.1;

	Vector3 center;
	Vector3 velocity;
	Vector3 force;

	Quaternion orientation;
	Vector3 angularVelocity;
	Vector3 torque;

	Matrix inverseInertiaTensor;

	OBB obb;

	RigidBodyComponent() = default;
	RigidBodyComponent(float mass, Vector3 center, Vector3 velocity, Vector3 angularVelocity,
					   Matrix inverseInertiaTensor, OBB obb)
		: mass(mass), center(center), velocity(velocity), orientation(QuaternionIdentity()),
		  angularVelocity(angularVelocity), inverseInertiaTensor(inverseInertiaTensor), obb(obb)
	{
		invMass = mass > 0.0f ? 1.0f / mass : 0.0f;
	}
	~RigidBodyComponent() = default;
	void intergrate(float deltaTime);
	Matrix getWorldInverseInertiaTensor() const;
};
