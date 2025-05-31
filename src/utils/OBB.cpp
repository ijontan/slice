#include "OBB.hpp"
#include "raylib.h"
#include "raymath.h"
#include <cfloat>
#include <cmath>

void projectOBB(const OBB &box, Vector3 axis, float &min, float &max)
{
	float centerProj = Vector3DotProduct(box.position, axis);
	float radius = box.halfSize.x * fabs(Vector3DotProduct(box.axis[0], axis)) +
				   box.halfSize.y * fabs(Vector3DotProduct(box.axis[1], axis)) +
				   box.halfSize.z * fabs(Vector3DotProduct(box.axis[2], axis));

	min = centerProj - radius;
	max = centerProj + radius;
}

bool overlapOnAxis(const OBB &a, const OBB &b, Vector3 axis, float &minPenetration, Vector3 &smallestAxis)
{
	if (Vector3Length(axis) < 0.0001f)
		return true; // skip near-zero axes

	axis = Vector3Normalize(axis);

	float minA, maxA;
	float minB, maxB;
	projectOBB(a, axis, minA, maxA);
	projectOBB(b, axis, minB, maxB);

	float overlap = fmin(maxA, maxB) - fmax(minA, minB);

	if (overlap < 0.0f)
		return false; // Separating axis found

	// Keep smallest penetration axis
	if (overlap < minPenetration)
	{
		minPenetration = overlap;
		smallestAxis = axis;
	}

	return true;
}

bool checkOBBCollision(const OBB &a, const OBB &b, Vector3 &contactNormal, float &penetration)
{
	penetration = FLT_MAX;
	Vector3 smallestAxis;

	Vector3 axes[15];
	int index = 0;

	// 3 axes from A
	axes[index++] = a.axis[0];
	axes[index++] = a.axis[1];
	axes[index++] = a.axis[2];

	// 3 axes from B
	axes[index++] = b.axis[0];
	axes[index++] = b.axis[1];
	axes[index++] = b.axis[2];

	// 9 cross products
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			axes[index++] = Vector3CrossProduct(a.axis[i], b.axis[j]);

	for (int i = 0; i < 15; i++)
		if (!overlapOnAxis(a, b, axes[i], penetration, smallestAxis))
			return false; // Separating axis found, no collision

	// Ensure normal points from A to B
	Vector3 centerDelta = Vector3Subtract(b.position, a.position);
	if (Vector3DotProduct(centerDelta, smallestAxis) < 0.0f)
		smallestAxis = Vector3Negate(smallestAxis);

	contactNormal = smallestAxis;
	return true; // All axes overlapped
}
