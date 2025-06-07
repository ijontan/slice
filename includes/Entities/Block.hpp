#pragma once
#include "Scene.hpp"
#include "math.hpp"
#include "raymath.h"

Entity setupBlock(Scene &scene, Vector3 position = {randomFloat(-100, 100), randomFloat(1, 100), randomFloat(-100, 100)},
				  Vector3 dimension = {randomFloat(1, 5), randomFloat(1, 5), randomFloat(1, 5)},
				  Vector3 angularVelocity = {randomFloat(), randomFloat(), randomFloat()},
				  Vector3 velocity = {0, 0, 0});
				  // Vector3 angularVelocity = {0, 0, 0},
				  // Vector3 velocity = {randomFloat(), randomFloat(), randomFloat()});
