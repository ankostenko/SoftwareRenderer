#include "internal.h"

#include "srmath.h"
#include "shader.h"
#include "color.h"
#include "image.h"
#include "model.h"

#include "timer.cpp"
#include "winlayer.h"

#include "renderer.h"
#include "renderer.cpp"

#include "camera.cpp"

#include "load.h"
#include "game.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../lib/stb_image_write.h"


bool globalRunning = true;
bool globalPause = false;

Model model;

int imageWidth = 1100;
int imageHeight = 600;

FlatShader flatShader;
LightShader lightShader;
PhongShader phongShader;
PhongShader asteroidShader;

int main(int argc, char **argv) {
	mouse = { imageWidth / 2, imageHeight / 2 };
	initRenderer(imageWidth, imageHeight, Vec3f({ 0.0f, 0.0f, 0.0f }));

	Model light;
	Model model1;
	Model bullet;
	Model asteroid;

	loadModel(model1, "..\\SoftwareRenderer\\models\\spaceship.obj");
	normalizeModelCoords(model1);
	loadModel(bullet, "..\\SoftwareRenderer\\models\\sphere.obj");
	normalizeModelCoords(bullet);
	loadModel(asteroid, "..\\SoftwareRenderer\\models\\asteroid.obj");
	normalizeModelCoords(asteroid);

	loadModel(light, "..\\SoftwareRenderer\\models\\sphere.obj");
	normalizeModelCoords(light);

	// TODO: default texture loading
	loadTexture(model, "models\\grid.jpg");

	clearImBuffer(black);

	HWND window = Win32CreateWindow(imageWidth, imageHeight, "3D Renderer");
	layer.window = window;
	Win32ShowCursor(false);
	SetCapture(window);

	float angleAlpha = 0;
	float angleBeta = 0;
	float angleGamma = 0;
	float cameraAngleAlpha = 0;
	float cameraAngleBeta = 0;
	float scaleVariable = 1.0f;
	int cameraForwardDirection = 0;
	int cameraRightDirection = 0;

	Timer fpsLock;
	Timer tm;

	// World coordinate system
	Vec3f X = { 1.0f, 0.0f, 0.0f };
	Vec3f Y = { 0.0f, 1.0f, 0.0f };
	Vec3f Z = { 0.0f, 0.0f, 1.0f };
	Vec3f origin = { 0.0f, 0.0f, 0.0f };

	PerspectiveCamera cameraP(0.1f, 1000.0f, (float)M_PI / 4);
	FreeCamera camera(0.1f, 100.0f, 45.0f, Vec3f({ 0.0f, 0.0f, 6.0f }));
	Mat4f shipTransform = { };

	std::vector<Bullet> bullets;
	std::vector<Asteroid> asteroids;
	asteroids.push_back(Asteroid({ true, Vec3f({ 1.0f, 0.0f, 0.0 }), 0, 0 }));
	asteroids.push_back(Asteroid({ true, Vec3f({ 0.0f, 0.0f, 0.0 }), 0, 0 }));
	asteroids.push_back(Asteroid({ true, Vec3f({ 0.0f, 0.0f, 0.0 }), 0, 0 }));
	asteroids.push_back(Asteroid({ true, Vec3f({ 0.0f, 0.0f, 0.0 }), 0, 0 }));
	asteroids.push_back(Asteroid({ true, Vec3f({ 0.0f, 0.0f, 0.0 }), 0, 0 }));
	asteroids.push_back(Asteroid({ true, Vec3f({ 0.0f, 0.0f, 0.0 }), 0, 0 }));
	asteroids.push_back(Asteroid({ true, Vec3f({ 0.0f, 0.0f, 0.0 }), 0, 0 }));
	asteroids.push_back(Asteroid({ true, Vec3f({ 0.0f, 0.0f, 0.0 }), 0, 0 }));
	asteroids.push_back(Asteroid({ true, Vec3f({ 0.0f, 0.0f, 0.0 }), 0, 0 }));
	asteroids.push_back(Asteroid({ true, Vec3f({ 0.0f, 0.0f, 0.0 }), 0, 0 }));

	std::default_random_engine generator;
	std::uniform_real_distribution<float> distribution(0, 1);
	
	int Score = 0;

	float lastFrame = 0.0f;
	while (globalRunning) {
		float currentFrame = fpsLock.secondsElapsed();
		float deltaTime = currentFrame - lastFrame;

		if (deltaTime < 0.033f) {
			std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(0.033f - currentFrame));
		}

		if (deltaTime > 0.033f) {
			tm.ResetStartTime();
			lastFrame = currentFrame;

			cameraForwardDirection = 0;
			cameraRightDirection = 0;
			layer.direction = 0;
			ProcessInput(window, angleBeta, deltaTime);

			clearZBuffer(camera.farClippingPlane);
			clearImBuffer(Color(56, 0, 0));

			camera.processMouseScrolling(mouse);
			camera.processMouseInput(mouse.x, mouse.y, deltaTime);
			camera.position = { 0.0f, 5.0f, 7.0f };
			camera.pitch = -40.0f;
			camera.yaw = 270.0f;
			camera.updateVectors();
			camera.lookAt();
			
			Mat4f vp = camera.view * camera.project();
			shipTransform = rotateY(angleBeta) * scale(3.0f) * translate(player.x, 0.0f, player.y);

			// World Coordinate system
			//drawLine(origin * vp, X * vp, red);
			//drawLine(origin * vp, Y * vp, green);
			//drawLine(origin * vp, Z * vp, blue);

			// Front vector
			player.front = norm(Vec3f({ 1.0f, 0.0f, 0.0f }) * rotateY(angleBeta));
			player.x += player.front.x * layer.direction * deltaTime * 3.5f;
			player.y += player.front.z * layer.direction * deltaTime * 3.5f;
			
			// "Border"
			if (player.x > 5.0f)  { player.x = -5.0f; }
			if (player.x < -5.0f) { player.x =  5.0f; }
			if (player.y > 3.0f)  { player.y = -4.0f; }
			if (player.y < -4.0f) { player.y =  3.0f; }

			// Draw vertical grid
			for (float index = -5; index < 6; index += 1.0f) {
				drawLine(Vec3f({ index, 0.0f, -4.0f }) * vp, Vec3f({ index, 0.0f, 3.0f }) * vp, white);
			}
			
			// Draw horizontal grid
			for (float index = -4; index < 4; index += 1.0f) {
				drawLine(Vec3f({ -5.0f, 0.0f, index }) * vp, Vec3f({ 5.0f, 0.0f, index }) * vp, white);
			}
			
			//drawLine(origin * vp, player.front * vp, magenta);

			// Bullet spawn	
			if (layer.shoot) {
				Bullet bl;
				bl.x = player.x;
				bl.y = player.y;
				
				bl.direction = player.front;
				bullets.push_back(bl);
			}

			for (int index = 0; index < bullets.size(); index++) {
				Bullet &bl = bullets[index];
				if (bl.x > 5.0f || bl.x < -5.0f || bl.y > 5.0f || bl.y < -5.0f) {
					bullets.erase(bullets.begin() + index);
				}
				bl.x = bl.x + bl.direction.x * deltaTime * 10.0f;
				bl.y = bl.y + bl.direction.z * deltaTime * 10.0f;
				Mat4f bulletTransform = translate(bl.x * 50, 0.0f, bl.y * 50) * scale(0.02f) * scaleY(0.1f);

				for (Asteroid &ast : asteroids) {
					if (!ast.available) {
						// Kill an asteroid
						if (abs(bl.x - ast.x / 5) < 0.2f && abs(bl.y - ast.y / 5) < 0.2f) {
							Score += 10;
							ast.available = true;
						}
					}
				}

				lightShader.uniform_MVP = bulletTransform * vp;
				lightShader.uniform_LightColor = { 0.0f, 255.0f, 255.0f };
				drawModel(bullet, lightShader);
			}

			// Light Movement
			Mat4f lightTransform = translate(0, 0, 0) * scale(0.2f);
			lightShader.uniform_MVP = lightTransform * vp;
			lightShader.uniform_LightColor = { 255.0f, 255.0f, 255.0f };
			drawModel(light, lightShader);

			// Asteroid's movement
			for (int index = 0; index < asteroids.size(); index++) {
				Asteroid &ast = asteroids[index];
				// Move an asteroid
				if (!ast.available) {
					// Player is dead
					//if (distanceBetweenPoints(Vec3f({ ast.x / 5, ast.y / 5, 0 }), Vec3f({ player.x, player.y, 0 })) < 0.125f) {
					//	player.x = 0;
					//	player.y = 0;
					//
					//	layer.lockCursor = false;
					//	Win32ShowCursor(true);
					//	LoseMenu(Score);
					//	Win32ShowCursor(false);
					//	layer.lockCursor = true;
					//	Score = 0;
					//	layer.heat = 0;
					//	for (Asteroid &ast : asteroids) {
					//		ast.available = true;
					//	}
					//}

					ast.x += ast.direction.x * deltaTime * 14.0f;
					ast.y += ast.direction.y * deltaTime * 14.0f;

					if (ast.x > 50.0f || ast.x < -50.0f || ast.y < -45.0f || ast.y > 30.0f) {
						ast.available = true;
					}

					Mat4f astTransform = translate(ast.x, 0.0f, ast.y) * scale(0.2f);

					//flatShader.uniform_M =  astTransform;
					//flatShader.uniform_MTI = inverse(astTransform);
					//flatShader.uniform_VP = vp;
					//flatShader.uniform_LightPos = render.light.position;
					// Phong
					asteroidShader.uniform_M = astTransform;
					asteroidShader.uniform_MTI = inverse(astTransform);
					asteroidShader.uniform_ObjColor = { 125.0f, 125.0f, 125.0f };
					asteroidShader.uniform_LightColor = { 1.0f, 1.0f, 1.0f };
					asteroidShader.uniform_ViewPos = camera.position;
					asteroidShader.uniform_LightPos = render.light.position;
					asteroidShader.uniform_VP = vp;
					drawModel(asteroid, asteroidShader);
				}
				else {
					// Generate "new" asteroid
					float distro = distribution(generator);
					ast.x = cos(distro * 2 * M_PI) * 35.0f;
					ast.y = sin(distro * 2 * M_PI) * 35.0f;

					if (ast.x > 0) {
						ast.direction.x = -distribution(generator);
					} else {
						ast.direction.x = distribution(generator);
					}
					if (ast.y > 0) {
						ast.direction.y = -distribution(generator);
					}
					else {
						ast.direction.y = distribution(generator);
					}

					ast.available = false;
				}
			}
			
			// Model shader
			phongShader.uniform_M = shipTransform;
			phongShader.uniform_MTI = inverse(shipTransform);
			phongShader.uniform_ObjColor = { 0.0f, 125.0f, 255.0f };
			phongShader.uniform_LightColor = { 1.0f, 1.0f, 1.0f };
			phongShader.uniform_ViewPos = camera.position;
			phongShader.uniform_LightPos = render.light.position;
			phongShader.uniform_VP = vp;
			drawModel(model1, phongShader);


			// UI system
			DrawRectangle(20, 10, 150, 40, white);
			DrawRectangle(20, 10, layer.heat, 40, Color(0, (1.0f - (float)layer.heat / (float)151) * 255, 255 * (float)layer.heat / (float)151));
			DrawRectangle(20, 10, 150, 3, peach);
			DrawRectangle(20, 10, 3,  40, peach);
			DrawRectangle(20, 47, 150, 3, peach);
			DrawRectangle(170,10, 3,  40, peach);

			DrawScore(render.imagebuffer.width / 2, 20, Score);
			// Millis
			DrawScore(40, render.imagebuffer.height - 50, (int)tm.milliElapsed());
			DrawScore(190, render.imagebuffer.height - 50, deltaTime * 1000);

			render.imagebuffer.flip_vertically();
			Win32DrawToWindow(window, render.imagebuffer.data, render.imagebuffer.width, render.imagebuffer.height);

			//char buffer[128];
			//sprintf(buffer, "%f ms Draw time: %f\n", deltaTime * 1000, tm.milliElapsed());
			//OutputDebugStringA(buffer);
		}
	}

	return 0;
}