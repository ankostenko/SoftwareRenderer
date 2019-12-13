#pragma once
#include "srmath.h"

struct Player {
	float x;
	float y;
	Vec3f front = { 1.0f, 0.0f, 0.0f };
};

struct Bullet {
	Vec3f direction;
	float x;
	float y;
};

struct Asteroid {
	bool available;
	Vec3f direction;
	float x;
	float y;
};

Player player;

const float ASTEROID_MOVEMENT_SPEED = 2.0f;