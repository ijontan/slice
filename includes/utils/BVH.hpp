#pragma once

#include "Scene.hpp"
#include "raylib.h"
#include <vector>

struct BVHNode
{
	BoundingBox bounds = {};
	BVHNode *left = nullptr;
	BVHNode *right = nullptr;
	std::vector<int> indexes;

	bool isLeaf()
	{
		return left == nullptr && right == nullptr;
	}
};

BoundingBox getModelBoundingBox(Model &model);

BVHNode *bvhBuild(std::vector<BoundingBox> &objects, std::vector<int> indexes, int depth = 0);
void bvhClean(BVHNode *headNode);
void bvhDetectCollision(BVHNode *node, BoundingBox &bound, std::vector<int> &result);
void bvhDisplay(BVHNode *node);
