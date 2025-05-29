#pragma once
#include "raymath.h"

struct OrientationComponent
{
	Quaternion orientation;

	OrientationComponent() = default;
	OrientationComponent(const OrientationComponent &) = default;
	OrientationComponent(const Quaternion &orientation) : orientation(orientation)
	{
	}
};
