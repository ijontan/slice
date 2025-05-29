#pragma once
#include "raylib.h"

struct OBB
{
	Vector3 position;
	Vector3 xAxis;
	Vector3 yAxis;
	Vector3 zAxis;
	Vector3 halfSize;
};

bool checkSeparatingPlane(const Vector3 &position, const Vector3 &plane, const OBB &a, const OBB &b);

// obb collision detection
bool checkCollision(const OBB &a, const OBB &b);
