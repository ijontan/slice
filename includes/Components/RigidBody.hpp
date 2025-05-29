#pragma once
#include "OBB.hpp"
#include "raymath.h"

struct RigidBodyComponent
{
	// Mesh mesh;
	float mass = 10;

	Vector3 center;
	Vector3 velocity;
	Vector3 force;

	Quaternion orientation;
	Vector3 angularVelocity;
	Vector3 torque;

	Matrix inverseInertiaTensor;

	OBB obb;

	RigidBodyComponent() = default;
	RigidBodyComponent(Vector3 center, Vector3 velocity, Vector3 angularVelocity, Matrix inverseInertiaTensor, OBB obb)
		: center(center), velocity(velocity), orientation(QuaternionIdentity()), angularVelocity(angularVelocity),
		  inverseInertiaTensor(inverseInertiaTensor), obb(obb)
	{
	}
	~RigidBodyComponent() = default;
	void Intergrate(float deltaTime);
};
