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

Matrix BlockFactory::computeInverseInertiaTensor() const
{
	// Dimensions are full extents, not half extents
	float w = dimension.x;
	float h = dimension.y;
	float d = dimension.z;

	float ix = (1.0f / 12.0f) * mass * (h * h + d * d);
	float iy = (1.0f / 12.0f) * mass * (w * w + d * d);
	float iz = (1.0f / 12.0f) * mass * (w * w + h * h);

	Matrix inertia = {
		ix, 0, 0, 0, 0, iy, 0, 0, 0, 0, iz, 0, 0, 0, 0, 1,
	};

	return MatrixInvert(inertia);
}

BlockFactory::BlockFactory(Scene &scene) : scene(scene)
{
	randomize();
}

void BlockFactory::randomize(){
	float mass = 10;
	setMass(mass);

	Vector3 position = {randomFloat(-100, 100), randomFloat(1, 100), randomFloat(-100, 100)};
	setPosition(position);

	Vector3 dimension = {randomFloat(1, 5), randomFloat(1, 5), randomFloat(1, 5)};
	setDimension(dimension);

	Vector3 velocity = {0, 0, 0};
	setVelocity(velocity);

	Vector3 angularVelocity = {randomFloat(), randomFloat(), randomFloat()};
	setAngularVelocity(angularVelocity);

	Quaternion orientation = QuaternionIdentity();
	setOrientation(orientation);

	CollisionMask category = CollisionMask::FREE;
	setCategory(category);

	unsigned int mask = CollisionMask::ALL;
	setCollisionMask(mask);

	Color color = WHITE;
	setColor(color);
}

Entity BlockFactory::generateBlock() const
{
	Entity block = scene.createEntity();

	// rigid body
	OBB obb = {};
	obb.halfSize = Vector3Scale(dimension, 0.5);
	RigidBodyComponent body(mass, position, velocity, angularVelocity, category, mask, computeInverseInertiaTensor(),
							obb, orientation);
	block.addComponent<RigidBodyComponent>(body);

	// Mesh
	Mesh mesh = GenMeshCube(dimension.x, dimension.y, dimension.z);
	Model model = LoadModelFromMesh(mesh);
	block.addComponent<BoxComponent>(model, color);

	return block;
}
