#include "BVH.hpp"
#include "Scene.hpp"
#include "raylib.h"
#include "raymath.h"
#include <iostream>
#include <numeric>

BoundingBox expandBoundingBox(BoundingBox &a, Vector3 &point)
{
	return {Vector3Min(a.min, point), Vector3Max(a.max, point)};
}

BoundingBox expandBoundingBox(BoundingBox &a, BoundingBox &b)
{
	return {Vector3Min(a.min, b.min), Vector3Max(a.max, b.max)};
}

BoundingBox expandBoundingBox(BoundingBox &a, float margin)
{
	Vector3 marginVector = {margin, margin, margin};
	return {Vector3Subtract(a.min, marginVector), Vector3Add(a.max, marginVector)};
}

Vector3 getBoundingBoxCenter(BoundingBox &a)
{
	return Vector3Scale(Vector3Add(a.min, a.max), 0.5f);
}

BVHNode *bvhBuild(std::vector<BoundingBox> &objects, std::vector<entt::entity> entities, int depth)
{
	BVHNode *node = new BVHNode;

	for (unsigned long i = 0; i < objects.size(); i++)
	{
		node->bounds = expandBoundingBox(node->bounds, objects[i]);
	}

	if (entities.size() <= 2 || depth >= 16)
	{
		node->entities = entities;
		return node;
	}

	// choose axis
	Vector3 extent = Vector3Subtract(node->bounds.max, node->bounds.min);
	int axis = 0;
	if (extent.y > extent.x && extent.y > extent.z)
		axis = 1;
	else if (extent.z > extent.x && extent.z > extent.y)
		axis = 2;

	std::vector<unsigned long> idx(entities.size());
	std::iota(idx.begin(), idx.end(), 0);
	// Sort by center of bounding box on chosen axis
	std::sort(idx.begin(), idx.end(), [&](int a, int b) {
		switch (axis)
		{
		case 0:
			return getBoundingBoxCenter(objects[a]).x < getBoundingBoxCenter(objects[b]).x;
		case 1:
			return getBoundingBoxCenter(objects[a]).y < getBoundingBoxCenter(objects[b]).y;
		case 2:
			return getBoundingBoxCenter(objects[a]).z < getBoundingBoxCenter(objects[b]).z;
		default:
			return true;
		}
	});

	unsigned long mid = idx.size() / 2;
	std::vector<entt::entity> left(mid);
	std::vector<entt::entity> right(mid + idx.size() % 2);
	std::vector<BoundingBox> leftObjects(mid);
	std::vector<BoundingBox> rightObjects(mid + idx.size() % 2);

	for (unsigned long i = 0; i < mid; i++)
	{
		left.push_back(entities[idx[i]]);
		right.push_back(entities[idx[i + mid]]);
		leftObjects.push_back(objects[idx[i]]);
		rightObjects.push_back(objects[idx[i + mid]]);
	}
	if (idx.size() % 2 == 1)
		right.push_back(*(--entities.end()));

	node->left = bvhBuild(leftObjects, left, ++depth);
	node->right = bvhBuild(rightObjects, right, ++depth);

	return node;
}

void bvhClean(BVHNode *node)
{
	if (node->isLeaf())
	{
		delete node;
		return;
	}
	bvhClean(node->left);
	bvhClean(node->right);
	delete node;
}

void bvhDetectCollision(BVHNode *node, BoundingBox &bound, std::vector<entt::entity> &result)
{
	if (!CheckCollisionBoxes(node->bounds, bound))
		return;

	if (node->isLeaf())
		for (auto entity : node->entities)
			result.push_back(entity);
	else
	{
		bvhDetectCollision(node->left, bound, result);
		bvhDetectCollision(node->right, bound, result);
	}
}
