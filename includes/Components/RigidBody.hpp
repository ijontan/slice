#pragma once
#include "OBB.hpp"
#include "raylib.h"
#include "raymath.h"

enum CollisionMask
{
	FREE = 0x0001,
	PLAYER = 0x0002,
	ENEMY = 0x0004,
	ALL = 0xFFFFFFFF,
	NONE = 0x0,
};

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

	CollisionMask category;
	unsigned int mask;

	Matrix inverseInertiaTensor;

	OBB obb;

	RigidBodyComponent() = default;
	RigidBodyComponent(float mass, Vector3 center, Vector3 velocity, Vector3 angularVelocity, CollisionMask category,
					   unsigned int mask, Matrix inverseInertiaTensor, OBB obb, Quaternion orientation)
		: mass(mass), center(center), velocity(velocity), orientation(orientation), angularVelocity(angularVelocity),
		  category(category), mask(mask), inverseInertiaTensor(inverseInertiaTensor), obb(obb)

	{
		invMass = mass > 0.0f ? 1.0f / mass : 0.0f;
	}
	~RigidBodyComponent() = default;
	void intergrate(float deltaTime);
	Matrix getWorldInverseInertiaTensor() const;
	bool shouldCollideWith(RigidBodyComponent &body)
	{
		if (!category || !body.category)
			return true;
		return (mask & body.category) != 0 && (body.mask & category) != 0;
	}
};
