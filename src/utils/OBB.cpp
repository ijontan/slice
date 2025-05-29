#include "OBB.hpp"
#include "raylib.h"
#include "raymath.h"
#include <cmath>

bool checkSeparatingPlane(const Vector3 &RPos, const Vector3 &Plane, const OBB &box1, const OBB &box2)
{
	return (fabs(Vector3DotProduct(RPos, Plane)) >
			(fabs(Vector3DotProduct(Vector3Scale(box1.xAxis, box1.halfSize.x), Plane)) +
			 fabs(Vector3DotProduct(Vector3Scale(box1.yAxis, box1.halfSize.y), Plane)) +
			 fabs(Vector3DotProduct(Vector3Scale(box1.zAxis, box1.halfSize.z), Plane)) +
			 fabs(Vector3DotProduct(Vector3Scale(box2.xAxis, box2.halfSize.x), Plane)) +
			 fabs(Vector3DotProduct(Vector3Scale(box2.yAxis, box2.halfSize.y), Plane)) +
			 fabs(Vector3DotProduct(Vector3Scale(box2.zAxis, box2.halfSize.z), Plane))));
}

// test for separating planes in all 15 axes
bool checkCollision(const OBB &a, const OBB &b)
{
	static Vector3 RPos;
	RPos = Vector3Subtract(b.position, a.position);

	return !(checkSeparatingPlane(RPos, a.xAxis, a, b) || checkSeparatingPlane(RPos, a.yAxis, a, b) ||
			 checkSeparatingPlane(RPos, a.zAxis, a, b) || checkSeparatingPlane(RPos, b.xAxis, a, b) ||
			 checkSeparatingPlane(RPos, b.yAxis, a, b) || checkSeparatingPlane(RPos, b.zAxis, a, b) ||
			 checkSeparatingPlane(RPos, Vector3CrossProduct(a.xAxis, b.xAxis), a, b) ||
			 checkSeparatingPlane(RPos, Vector3CrossProduct(a.xAxis, b.yAxis), a, b) ||
			 checkSeparatingPlane(RPos, Vector3CrossProduct(a.xAxis, b.zAxis), a, b) ||
			 checkSeparatingPlane(RPos, Vector3CrossProduct(a.yAxis, b.xAxis), a, b) ||
			 checkSeparatingPlane(RPos, Vector3CrossProduct(a.yAxis, b.yAxis), a, b) ||
			 checkSeparatingPlane(RPos, Vector3CrossProduct(a.yAxis, b.zAxis), a, b) ||
			 checkSeparatingPlane(RPos, Vector3CrossProduct(a.zAxis, b.xAxis), a, b) ||
			 checkSeparatingPlane(RPos, Vector3CrossProduct(a.zAxis, b.yAxis), a, b) ||
			 checkSeparatingPlane(RPos, Vector3CrossProduct(a.zAxis, b.zAxis), a, b));
}
