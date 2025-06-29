
#pragma once
#include "raylib.h"

struct OBB
{
	Vector3 position;
	Vector3 axis[3];
	Vector3 halfSize;
};

// obb collision detection
Vector3 getOBBVertex(const OBB &obb, int index);
bool checkOBBCollision(const OBB &a, const OBB &b, Vector3 &contactNormal, float &penetration);
