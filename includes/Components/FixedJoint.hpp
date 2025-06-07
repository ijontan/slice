#pragma once

#include "Entity.hpp"

struct FixedJoint
{
	Entity a;
	Entity b;

	Vector3 localAnchorA;
	Vector3 localAnchorB;

	Quaternion initialRotationOffset;
};

