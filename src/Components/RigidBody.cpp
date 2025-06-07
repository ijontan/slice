
#include "RigidBody.hpp"
#include "raylib.h"
#include "raymath.h"
#include <cmath>

void RigidBodyComponent::intergrate(float deltaTime)
{
	if (invMass <= 0.0f)
		return;

	// --- Linear Motion ---
	Vector3 acceleration = Vector3Scale(force, invMass);
	velocity = Vector3Add(velocity, Vector3Scale(acceleration, deltaTime));

	// Time-based damping
	float linearDampingFactor = 0.1f;
	float damping = powf(1.0f - linearDampingFactor, deltaTime);
	velocity = Vector3Scale(velocity, damping);

	center = Vector3Add(center, Vector3Scale(velocity, deltaTime));
	force = (Vector3){0.0f, 0.0f, 0.0f};
	if (Vector3LengthSqr(velocity) < 0.001f)
		velocity = {0, 0, 0};

	// --- Angular Motion ---
	Matrix worldInvInertia = getWorldInverseInertiaTensor();
	Vector3 angularAcceleration = Vector3Transform(torque, worldInvInertia);
	angularVelocity = Vector3Add(angularVelocity, Vector3Scale(angularAcceleration, deltaTime));

	float angularDampingFactor = 0.01f;
	float angularDamping = powf(1.0f - angularDampingFactor, deltaTime);
	angularVelocity = Vector3Scale(angularVelocity, angularDamping);
	if (Vector3LengthSqr(angularVelocity) < 0.00001f)
		angularVelocity = {0, 0, 0};

	// --- Update Orientation ---
	Quaternion delta =
		QuaternionMultiply((Quaternion){angularVelocity.x, angularVelocity.y, angularVelocity.z, 0.0f}, orientation);
	delta = QuaternionScale(delta, 0.5f * deltaTime);
	orientation = QuaternionNormalize(QuaternionAdd(orientation, delta));

	torque = (Vector3){0.0f, 0.0f, 0.0f};
}

Matrix RigidBodyComponent::getWorldInverseInertiaTensor() const
{
	Matrix rot = QuaternionToMatrix(orientation);
	Matrix rotT = MatrixTranspose(rot);
	return MatrixMultiply(MatrixMultiply(rot, inverseInertiaTensor), rotT);
}
