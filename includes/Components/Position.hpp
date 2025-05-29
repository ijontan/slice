#pragma once
#include "raymath.h"

struct PositionComponent
{
	Vector3 value;

	PositionComponent() = default;
	~PositionComponent() = default;
	PositionComponent(const PositionComponent &) = default;
	PositionComponent(const Vector3 &value) : value(value)
	{
	}

	operator const Vector3 &() const
	{
		return value;
	}
	operator Vector3 &()
	{
		return value;
	}
};
