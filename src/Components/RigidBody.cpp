
#include "RigidBody.hpp"
#include "raymath.h"

void RigidBodyComponent::Intergrate(float deltaTime)
{
	if (invMass <= 0.0f)
		return;
	// Linear motion
	Vector3 acceleration = Vector3Add(Vector3Scale(this->force, this->invMass), {0, -10, 0});
	this->velocity = Vector3Add(this->velocity, Vector3Scale(acceleration, deltaTime));
	this->center = Vector3Add(this->center, Vector3Scale(this->velocity, deltaTime));
	this->force = (Vector3){0.0f, 0.0f, 0.0f}; // Reset force
	if (Vector3LengthSqr(this->velocity) < 0.001f)
		this->velocity = {0, 0, 0};

	// Angular motion
	Vector3 angularAcceleration = Vector3Transform(this->torque, this->inverseInertiaTensor);
	this->angularVelocity = Vector3Add(this->angularVelocity, Vector3Scale(angularAcceleration, deltaTime));
	if (Vector3LengthSqr(this->angularVelocity) < 0.001f)
		this->velocity = {0, 0, 0};

	// Update orientation using quaternion derivative
	Quaternion omega = (Quaternion){0.0f, this->angularVelocity.x, this->angularVelocity.y, this->angularVelocity.z};
	Quaternion deltaOrientation = QuaternionMultiply(QuaternionScale(omega, 0.5f * deltaTime), this->orientation);
	this->orientation = QuaternionNormalize(QuaternionAdd(this->orientation, deltaOrientation));
	this->torque =
		(Vector3){0.0f, 0.0f, 0.0f}; // Reset torque   this->torque = (Vector3){0.0f, 0.0f, 0.0f}; // Reset torque
}
