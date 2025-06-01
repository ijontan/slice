#pragma once
#include "Scene.hpp"

struct PlayerState {
	bool camLock;
	bool active;
};

void createPlayer(Scene &scene, bool active);
