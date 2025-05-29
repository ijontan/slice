#pragma once

#include "Scene.hpp"
#include "raylib.h"
#include <vector>

struct BVHNode
{
	BoundingBox bounds;
	BVHNode *left = nullptr;
	BVHNode *right = nullptr;
	std::vector<entt::entity> entities;

	bool isLeaf()
	{
		return left == nullptr && right == nullptr;
	}
};

BVHNode *bvhBuild(std::vector<BoundingBox> &objects, std::vector<entt::entity> entities, int depth = 0);
void bvhClean(BVHNode *headNode);
void bvhDetectCollision(BVHNode *node, BoundingBox &bound, std::vector<entt::entity> &result);
