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

Player player;
const int MOVEMENT_SPEED = 0.9f;

void UpdatePosition(int direction) {
	player.x += player.front.x * direction * MOVEMENT_SPEED;
	player.y += player.front.y * direction * MOVEMENT_SPEED;
}

void Game() {
	UpdatePosition(layer.direction);
}