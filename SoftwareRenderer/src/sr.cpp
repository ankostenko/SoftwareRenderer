#include "internal.h"

#include "srmath.h"
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
//OrthographicCamera camera(0.1, 1000.0f, (float)M_PI / 3);

int imageWidth =  800;
int imageHeight = 600;

int main(int argc, char **argv) {
	mouse = { imageWidth / 2, imageHeight / 2 };
	initRenderer(imageWidth, imageHeight, Vec3f({ 0.0f,  2.5f, 5.0f }));

	loadModel(model, "models\\cube.obj");
	normalizeModelCoords(model);

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

	// World coordinate system
	Vec3f X = { 1.0f, 0.0f, 0.0f };
	Vec3f Y = { 0.0f, 1.0f, 0.0f };
	Vec3f Z = { 0.0f, 0.0f, 1.0f };
	Vec3f origin = { 0.0f, 0.0f, 0.0f };

	PerspectiveCamera cameraP(0.1f, 1000.0f, (float)M_PI / 3);
	FreeCamera camera(0.1f, 100.0f, 45.0f, Vec3f({ 0.0f, 0.0f, 6.0f }));

	while (globalRunning) {
		float timeEllapsed = fpsLock.milliElapsed();

		if (timeEllapsed < 16.0f) {
			std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(16.0f - timeEllapsed));
		}
		fpsLock.ResetStartTime();
		float deltaTime = (timeEllapsed - fpsLock.milliElapsed()) / 33;

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

		Vec3f rOrigin = origin * vp;

		drawLine(rOrigin, X * vp, red);
		drawLine(rOrigin, Y * vp, green);
		drawLine(rOrigin, Z * vp, blue);

		Mat4f modelTransform = translate(0.0f, 0.0f, 0.0f);
		Mat4f proj = camera.project();
		drawModel(model, modelTransform, camera.view, camera.proj);

		render.imagebuffer.flip_vertically();
		Win32DrawToWindow(window, render.imagebuffer.data, render.imagebuffer.width, render.imagebuffer.height);

		char buffer[64];
		wsprintf(buffer, "%d ms - %d\n", (int)fpsLock.milliElapsed(), int(scaleVariable * 10));
		OutputDebugStringA(buffer);
	}

	return 0;
}