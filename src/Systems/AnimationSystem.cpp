#include "AnimationSystem.hpp"
#include "Shape.hpp"
#include "raymath.h"
#include <iostream>
#include <ostream>

void animateBlockSpawn(entt::registry &registry)
{
	auto view = registry.view<BoxComponent>();

	for (auto entity : view)
	{
		auto &box = registry.get<BoxComponent>(entity);
		if (box.scale > 1.0f)
			continue;
		box.scale += 0.05f;
	}
}
