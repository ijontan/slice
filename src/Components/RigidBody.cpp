
#include "RigidBody.hpp"
#include "raylib.h"
#include "raymath.h"
#include <cmath>

void RigidBodyComponent::Intergrate(float deltaTime)
{
	if (invMass <= 0.0f)
		return;

	// --- Apply Gravity ---
	const Vector3 gravity = {0.0f, -9.81f, 0.0f};
	if (mass > 0)
		force = Vector3Add(force, Vector3Scale(gravity, mass));

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

	float angularDampingFactor = 0.1f;
	float angularDamping = powf(1.0f - angularDampingFactor, deltaTime);
	angularVelocity = Vector3Scale(angularVelocity, angularDamping);
	if (Vector3LengthSqr(angularVelocity) < 0.001f)
		angularVelocity = {0, 0, 0};

	// --- Update Orientation ---
	Quaternion omega = {0.0f, angularVelocity.x, angularVelocity.y, angularVelocity.z};
	Quaternion deltaOrientation = QuaternionMultiply(QuaternionScale(omega, 0.5f * deltaTime), orientation);
	orientation = QuaternionNormalize(QuaternionAdd(orientation, deltaOrientation));

	torque = (Vector3){0.0f, 0.0f, 0.0f};
}

Matrix RigidBodyComponent::getWorldInverseInertiaTensor() const
{
	Matrix rot = QuaternionToMatrix(orientation);
	Matrix rotT = MatrixTranspose(rot);
	return MatrixMultiply(MatrixMultiply(rot, inverseInertiaTensor), rotT);
}
