#pragma once
#include "RigidBody.hpp"
#include "Scene.hpp"
#include "raylib.h"

class BlockFactory
{
public:
	BlockFactory(Scene &scene);

	void setMass(float mass)
	{
		this->mass = mass;
	}
	void setPosition(const Vector3 &position)
	{
		this->position = position;
	}
	void setDimension(const Vector3 &dimension)
	{
		this->dimension = dimension;
	}
	void setVelocity(const Vector3 &velocity)
	{
		this->velocity = velocity;
	}
	void setAngularVelocity(const Vector3 &angularVelocity)
	{
		this->angularVelocity = angularVelocity;
	}
	void setOrientation(const Quaternion &orientation)
	{
		this->orientation = orientation;
	}
	void setCategory(CollisionMask category = CollisionMask::FREE)
	{
		this->category = category;
	}
	void setCollisionMask(unsigned int mask = 0xffffffff)
	{
		this->mask = mask;
	}
	void setColor(const Color &color)
	{
		this->color = color;
	}

	void randomize();
	Entity generateBlock() const;

private:
	Scene &scene;
	float mass;
	Vector3 position;
	Vector3 dimension;
	Vector3 velocity;
	Vector3 angularVelocity;
	Quaternion orientation;
	CollisionMask category;
	unsigned int mask;
	Color color;

	Matrix computeInverseInertiaTensor() const;
};
