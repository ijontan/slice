// #include "Position.hpp"
#include "Block.hpp"
#include "Entity.hpp"
#include "OBB.hpp"
#include "Position.hpp"
#include "Scene.hpp"
// #include "raymath.h"
#include "Components/Shape.hpp"
#include "RigidBody.hpp"
#include "math.hpp"
#include "raylib.h"
#include "raymath.h"

Matrix ComputeInverseInertiaTensor(float mass, Vector3 dimension)
{
	// Dimensions are full extents, not half extents
	float w = dimension.x;
	float h = dimension.y;
	float d = dimension.z;

	float ix = (1.0f / 12.0f) * mass * (h * h + d * d);
	float iy = (1.0f / 12.0f) * mass * (w * w + d * d);
	float iz = (1.0f / 12.0f) * mass * (w * w + h * h);

	Matrix inertia = {
		ix, 0,  0,  0,
		0,  iy, 0,  0,
		0,  0,  iz, 0,
		0,  0,  0,  1,
	};

	return MatrixInvert(inertia);
}

Entity setupBlock(Scene &scene, Vector3 position, Vector3 dimension, Vector3 angularVelocity, Vector3 velocity)
{
	Entity block = scene.createEntity();

	OBB obb = {};
	obb.halfSize = Vector3Scale(dimension, 0.5);
	block.addComponent<RigidBodyComponent>(10, position, Vector3Scale(velocity, 10), angularVelocity,
										   ComputeInverseInertiaTensor(10, dimension), obb);

	Mesh mesh = GenMeshCube(dimension.x, dimension.y, dimension.z);
	Model model = LoadModelFromMesh(mesh);
	block.addComponent<BoxComponent>(model);
	return block;
}
