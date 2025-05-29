// #include "Position.hpp"
#include "Entity.hpp"
#include "OBB.hpp"
#include "Position.hpp"
#include "Scene.hpp"
// #include "raymath.h"
#include "Components/Shape.hpp"
#include "RigidBody.hpp"
#include "raylib.h"
#include "raymath.h"

float randomFloat(float min = -1.0f, float max = 1.0f)
{
	return min + static_cast<float>(std::rand()) / (static_cast<float>(RAND_MAX) / (max - min));
}

Matrix ComputeInverseInertiaTensor(float mass, Vector3 dimension)
{
	float factor = 12.0f / mass;
	float ixx = factor / (dimension.x * dimension.x + dimension.z * dimension.z);
	float iyy = factor / (dimension.y * dimension.y + dimension.z * dimension.z);
	float izz = factor / (dimension.y * dimension.y + dimension.x * dimension.x);

	Matrix inverseInertia = {ixx,  0.0f, 0.0f, 0.0f, 0.0f, iyy,	 0.0f, 0.0f,
							 0.0f, 0.0f, izz,  0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
	return inverseInertia;
}

void setupBlock(Scene &scene)
{
	Entity block = scene.createEntity();

	// Vector3 position = {randomFloat(-1, 1), randomFloat(1.5, 10), randomFloat(-1, 1)};
	Vector3 position = {randomFloat(-50, 50), randomFloat(1, 20), randomFloat(-50, 50)};
	// Vector3 velocity = {randomFloat(), randomFloat(), randomFloat()};
	Vector3 angularVelocity = {randomFloat(), randomFloat(), randomFloat()};
	Vector3 velocity = {0, 0, 0};
	// block.addComponent<RigidBodyComponent>(position, Vector3Scale(velocity, 10), ComputeInverseInertiaTensor(10, 1,
	// 1, 1));
	Vector3 dimension = {randomFloat(1, 2), randomFloat(1, 2), randomFloat(1, 2)};
	OBB obb = {};
	obb.halfSize = Vector3Scale(dimension, 0.5);
	block.addComponent<RigidBodyComponent>(position, Vector3Scale(velocity, 10), angularVelocity,
										   ComputeInverseInertiaTensor(10, dimension), obb);

	// Vector3 dimension = {1, 1, 1};
	Mesh mesh = GenMeshCube(dimension.x, dimension.y, dimension.z);
	Model model = LoadModelFromMesh(mesh);
	block.addComponent<BoxComponent>(model);
}
