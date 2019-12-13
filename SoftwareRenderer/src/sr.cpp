#include "internal.h"

#include "srmath.h"
#include "shader.h"
#include "color.h"
#include "image.h"
#include "model.h"

#include "winlayer.h"
#include "timer.cpp"

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

int main(int argc, char **argv) {
	mouse = { imageWidth / 2, imageHeight / 2 };
	initRenderer(imageWidth, imageHeight, Vec3f({ 0.0f, 0.0f, -12.0f * 50 }));

	Model light;
	Model model1;
	Model bullet;
	Model asteroid;

	loadModel(model1, "models\\spaceship.obj");
	normalizeModelCoords(model1);
	loadModel(bullet, "models\\sphere.obj");
	normalizeModelCoords(bullet);
	loadModel(asteroid, "models\\asteroid.obj");
	normalizeModelCoords(asteroid);

	loadModel(light, "models\\sphere.obj");
	//normalizeModelCoords(light);

	// TODO: default texture loading
	loadTexture(model, "models\\grid.jpg");

	clearImBuffer(black);

	HWND window = Win32CreateWindow(imageWidth, imageHeight, "3D Renderer");
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
	asteroids.push_back(Asteroid({ false, Vec3f({ 0.0f, 0.0f, 0.0 }), 0, -4 * 12 }));
	asteroids.push_back(Asteroid({ false, Vec3f({ 0.0f, 0.0f, 0.0 }), 0, -4 * 12 }));
	asteroids.push_back(Asteroid({ false, Vec3f({ 0.0f, 0.0f, 0.0 }), 0, -4 * 12 }));
	asteroids.push_back(Asteroid({ false, Vec3f({ 0.0f, 0.0f, 0.0 }), 0, -4 * 12 }));
	asteroids.push_back(Asteroid({ false, Vec3f({ 0.0f, 0.0f, 0.0 }), 0, -4 * 12 }));
	asteroids.push_back(Asteroid({ false, Vec3f({ 0.0f, 0.0f, 0.0 }), 0, -4 * 12 }));
	asteroids.push_back(Asteroid({ false, Vec3f({ 0.0f, 0.0f, 0.0 }), 0, -4 * 12 }));
	asteroids.push_back(Asteroid({ false, Vec3f({ 0.0f, 0.0f, 0.0 }), 0, -4 * 12 }));
	asteroids.push_back(Asteroid({ false, Vec3f({ 0.0f, 0.0f, 0.0 }), 0, -4 * 12 }));
	asteroids.push_back(Asteroid({ false, Vec3f({ 0.0f, 0.0f, 0.0 }), 0, -4 * 12 }));

	std::default_random_engine generator;
	std::uniform_real_distribution<float> distribution(0, 1);
	const std::array<int, 2> posList = { -1, 1 };
	std::random_device rd;
	std::mt19937 eng(rd());
	std::uniform_int_distribution<> distr(0, posList.size() - 1);
	
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
			ProcessInput(window, angleAlpha, angleBeta, angleGamma, cameraForwardDirection, cameraRightDirection, scaleVariable, deltaTime);

			clearZBuffer(camera.farClippingPlane);
			//clearImBuffer(Color(255 * 0.2f, 255 * 0.1f, 255 * 0.0f));
			clearImBuffer(Color(56, 0, 0));

			camera.processMouseScrolling(mouse);
			camera.processMouseInput(mouse.x, mouse.y, deltaTime);
			camera.position = { 0.0f, 5.0f, 7.0f };
			camera.pitch = -40.0f;
			camera.yaw = 270.0f;
			camera.updateVectors();
			camera.lookAt();
			
			Mat4f vp = camera.view * camera.project();
			shipTransform = transpose(rotateY(angleBeta)) * scale(3.0f) * translate(player.x, 0.0f, player.y);

			// World Coordinate system
			//drawLine(origin * vp, X * vp, red);
			//drawLine(origin * vp, Y * vp, green);
			//drawLine(origin * vp, Z * vp, blue);

			// Front vector
			player.front = norm(Vec3f({ 1.0f, 0.0f, 0.0f }) * transpose(rotateY(angleBeta)));
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
				Mat4f bulletTransform = translate(bl.x * 50, 0.0f, bl.y * 50) * scale(0.02f) * scaleY(0.3f);

				lightShader.uniform_MVP = bulletTransform * vp;
				lightShader.uniform_LightColor = { 0.0f, 255.0f, 255.0f };
				drawModel(bullet, lightShader);
			}

			// Light Movement
			Mat4f lightTransform = translate(render.light.position.x, render.light.position.y, render.light.position.z) * scale(0.02f);
			lightShader.uniform_MVP = lightTransform * vp;
			lightShader.uniform_LightColor = { 255.0f, 255.0f, 255.0f };
			drawModel(light, lightShader);

			// Asteroid's movement
			for (int index = 0; index < asteroids.size(); index++) {
				Asteroid &ast = asteroids[index];
				// Move an asteroid
				if (!ast.available) {
					ast.x += ast.direction.x * deltaTime * 14.0f;
					ast.y += ast.direction.y * deltaTime * 14.0f;

					if (ast.x > 50.0f || ast.x < -50.0f || ast.y < -45.0f || ast.y > 30.0f) {
						ast.available = true;
					}

					Mat4f astTransform = translate(ast.x, 0.0f, ast.y) * scale(0.2f);

					flatShader.uniform_M = astTransform;
					flatShader.uniform_VP = vp;
					flatShader.uniform_LightPos = render.light.position;
					// Phong
					phongShader.uniform_M = astTransform;
					phongShader.uniform_MTI = transpose(inverse(astTransform));
					phongShader.uniform_ObjColor = { 125.0f, 125.0f, 125.0f };
					phongShader.uniform_LightColor = { 1.0f, 1.0f, 1.0f };
					phongShader.uniform_ViewPos = camera.position;
					phongShader.uniform_LightPos = render.light.position;
					phongShader.uniform_VP = vp;
					drawModel(asteroid, phongShader);
				}
				else {
					// Generate "new" asteroid
					ast.x = (distribution(generator)) * posList[distr(eng)] * 40.0f;
					ast.y = (distribution(generator)) * posList[distr(eng)] * 40.0f;

					if (ast.x > 0) {
						ast.direction.x = -distribution(generator);
					}
					else {
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
			phongShader.uniform_MTI = transpose(inverse(shipTransform));
			phongShader.uniform_ObjColor = { 0.0f, 125.0f, 255.0f };
			phongShader.uniform_LightColor = { 1.0f, 1.0f, 1.0f };
			phongShader.uniform_ViewPos = camera.position;
			phongShader.uniform_LightPos = render.light.position;
			phongShader.uniform_VP = vp;
			drawModel(model1, phongShader);

			render.imagebuffer.flip_vertically();
			Win32DrawToWindow(window, render.imagebuffer.data, render.imagebuffer.width, render.imagebuffer.height);

			char buffer[128];
			sprintf(buffer, "%f ms Draw time: %f Yaw: %f Front: %f, %f, %f\n", deltaTime * 1000, tm.milliElapsed(), layer.yaw,
					player.front.x, player.front.y, player.front.z);
			OutputDebugStringA(buffer);
		}
	}

	return 0;
}