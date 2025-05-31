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

BoundingBox getModelBoundingBox(Model &model)
{

	BoundingBox bb;
	Mesh &mesh = model.meshes[0];

	Vector3 *vector = reinterpret_cast<Vector3 *>(mesh.vertices);
	Vector3 p1 = Vector3Transform(*vector, model.transform);
	bb = {p1, p1};
	for (int i = 3; i < mesh.vertexCount * 3; i += 3)
	{
		Vector3 *vector = reinterpret_cast<Vector3 *>(mesh.vertices + i);

		Vector3 p = Vector3Transform(*vector, model.transform);
		bb = expandBoundingBox(bb, p);
	}
	return bb;
}

BVHNode *bvhBuild(std::vector<BoundingBox> &objects, std::vector<int> indexes, int depth)
{
	BVHNode *node = new BVHNode;

	if (indexes.size() <= 2 || depth >= 16)
	{
		bool first = true;
		for (int i : indexes)
		{
			if (first)
			{
				node->bounds = objects[i];
				first = false;
				continue;
			}
			node->bounds = expandBoundingBox(node->bounds, objects[i]);
		}
		node->indexes = indexes;
		return node;
	}

	// choose axis
	Vector3 extent = Vector3Subtract(node->bounds.max, node->bounds.min);
	int axis = 0;
	if (extent.y > extent.x && extent.y > extent.z)
		axis = 1;
	else if (extent.z > extent.x && extent.z > extent.y)
		axis = 2;

	// Sort by center of bounding box on chosen axis
	std::sort(indexes.begin(), indexes.end(), [&](int a, int b) {
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

	unsigned long mid = indexes.size() / 2;
	std::vector<int> left(indexes.begin(), indexes.begin() + mid);
	std::vector<int> right(indexes.begin() + mid, indexes.end());

	node->left = bvhBuild(objects, left, ++depth);
	node->right = bvhBuild(objects, right, ++depth);

	node->bounds = node->left->bounds;
	node->bounds = expandBoundingBox(node->bounds, node->right->bounds);
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

void bvhDetectCollision(BVHNode *node, BoundingBox &bound, std::vector<int> &result)
{
	if (!CheckCollisionBoxes(node->bounds, bound))
		return;

	if (node->isLeaf())
		for (auto entity : node->indexes)
			result.push_back(entity);
	else
	{
		bvhDetectCollision(node->left, bound, result);
		bvhDetectCollision(node->right, bound, result);
	}
}

void bvhDisplay(BVHNode *node)
{
	DrawBoundingBox(node->bounds, BLACK);
	if (node->isLeaf())
	{
		return;
	}
	bvhDisplay(node->left);
	bvhDisplay(node->right);
}
