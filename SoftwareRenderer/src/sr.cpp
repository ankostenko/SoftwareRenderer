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

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../lib/stb_image_write.h"


bool globalRunning = true;
bool globalPause = false;

Model model;

int imageWidth = 800;
int imageHeight = 600;

FlatShader flatShader;
LightShader lightShader;
PhongShader phongShader;

int main(int argc, char **argv) {
	mouse = { imageWidth / 2, imageHeight / 2 };
	initRenderer(imageWidth, imageHeight, Vec3f({ 0.0f,  0.0f, 5.0f }));

	Model light;

	loadModel(model, "models\\teapot.obj");
	loadModel(light, "models\\sphere.obj");
	normalizeModelCoords(model);
	normalizeModelCoords(light);

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

	PerspectiveCamera cameraP(0.1f, 1000.0f, (float)M_PI / 3);
	FreeCamera camera(0.1f, 100.0f, 45.0f, Vec3f({ 0.0f, 0.0f, 6.0f }));

	float lastFrame = 0.0f;
	while (globalRunning) {
		float currentFrame = fpsLock.secondsElapsed();
		float deltaTime = currentFrame - lastFrame;

		if (deltaTime > 0.033f) {
			tm.ResetStartTime();
			//std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(16.0f - timeEllapsed));
			lastFrame = currentFrame;

			cameraForwardDirection = 0;
			cameraRightDirection = 0;
			ProcessInput(window, angleAlpha, angleBeta, angleGamma, cameraForwardDirection, cameraRightDirection, scaleVariable, deltaTime);

			clearZBuffer(camera.farClippingPlane);
			clearImBuffer(Color(255 * 0.6f, 255 * 0.3f, 255 * 0.2f));

			camera.processMouseInput(mouse.x, mouse.y, deltaTime);
			camera.processMouseScrolling(mouse);
			camera.forwardMovement(cameraForwardDirection, deltaTime);
			camera.rightMovement(cameraRightDirection, deltaTime); 
			camera.updateVectors();
			camera.lookAt();

			Mat4f vp = camera.view * camera.project();

			//Vec3f rOrigin = origin * vp;
			//drawLine(rOrigin, X * vp, red);
			//drawLine(rOrigin, Y * vp, green);
			//drawLine(rOrigin, Z * vp, blue);

			Mat4f modelTransform = translate(0.0f, 0.0f, 0.0f) * rotate(angleAlpha, 0, 0);
			Mat4f proj = camera.project();

			// Light Movement
			render.light.position.y = 7.0f;
			render.light.position.x = 200 * sin(fpsLock.secondsElapsed() / 2) * deltaTime;
			render.light.position.z = 200 * cos(fpsLock.secondsElapsed() / 2) * deltaTime;
			Mat4f lightTransform = translate(render.light.position.x, render.light.position.y, render.light.position.z) * scale(0.2f);
			lightShader.uniform_MVP = lightTransform * vp;
			drawModel(light, lightShader); 
			
			// Model shader			
			//flatShader.uniform_VP = vp;
			//flatShader.uniform_M = modelTransform;
			//drawModel(model, flatShader); 

			phongShader.uniform_LightColor = { 1.0f, 1.0f, 1.0f };
			phongShader.uniform_M = modelTransform;
			phongShader.uniform_MTI = transpose(inverse(modelTransform));
			phongShader.uniform_VP = vp;
			phongShader.uniform_ObjColor = { 0.0f, 102.0f, 204.0f };
			phongShader.uniform_LightPos = render.light.position;
			drawModel(model, phongShader);

			render.imagebuffer.flip_vertically();
			Win32DrawToWindow(window, render.imagebuffer.data, render.imagebuffer.width, render.imagebuffer.height);
			

			char buffer[64];
			sprintf(buffer, "%f ms Draw time: %f\n", deltaTime * 1000, tm.milliElapsed());
			OutputDebugStringA(buffer);
		}
	}

	return 0;
}