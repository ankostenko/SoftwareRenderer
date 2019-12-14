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


void LoseMenu(int finalScore) {
	Timer menuTimer;
	float _placeholder;
	float lastFrame = 0;
	while (globalRunning) {
		float currentFrame = menuTimer.secondsElapsed();
		float deltaTime = currentFrame - lastFrame;

		if (deltaTime > 0.033f) {
			lastFrame = currentFrame;
			clearImBuffer(Color(56, 0, 0));
			
			ProcessInput(layer.window, _placeholder, 0);
			DrawScore(render.imagebuffer.width / 2, render.imagebuffer.height / 2 - 100, finalScore);
			
			render.imagebuffer.flip_vertically();
			Win32DrawToWindow(layer.window, render.imagebuffer.data, render.imagebuffer.width, render.imagebuffer.height);
		}
	}
}