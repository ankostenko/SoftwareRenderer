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
PerspectiveCamera camera(0.1f, 1000.0f, (float)M_PI / 3);
//OrthographicCamera camera(0.1, 1000.0f, (float)M_PI / 3);

int imageWidth = 1300;
int imageHeight = 700;

int main(int argc, char **argv) {
	initRenderer(imageWidth, imageHeight);

	loadModel(model, "models\\african_head.obj");
	normalizeModelCoords(model);

	// TODO: default texture loading
	loadTexture(model, "models\\african_head_diffuse.jpg");

	clearImBuffer(black);

	HWND window = Win32CreateWindow(imageWidth, imageHeight, "3D Renderer");

	float angleTheta = 0;
	float anglePhi = 0;
	float cameraAngleTheta = 0;
	float cameraAnglePhi = 0;
	float scaleVariable = 1.0f;

	Timer fpsLock;

	// World coordinate system
	Vec3f X = { 1.0f, 0.0f, 0.0f };
	Vec3f Y = { 0.0f, 1.0f, 0.0f };
	Vec3f Z = { 0.0f, 0.0f, 1.0f };
	Vec3f origin = { 0.0f, 0.0f, 0.0f };

	while (globalRunning) {
		float timeEllapsed = fpsLock.milliElapsed();

		if (timeEllapsed < 16.0f) {
			std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(16.0f - timeEllapsed));
		}
		fpsLock.ResetStartTime();
		float deltaTime = (timeEllapsed - fpsLock.milliElapsed()) / 33;

		ProcessInput(window, angleTheta, anglePhi, cameraAngleTheta, cameraAnglePhi, scaleVariable, deltaTime);

		clearZBuffer(camera.farClippingPlane);
		clearImBuffer(peach);

		// Camera
		camera.position = camera.staticPosition * translate(0.0f, 0.0f, 1.5f) * rotationX(cameraAnglePhi) * rotationY(cameraAngleTheta);
		camera.lookAt(origin);
		// NOTE: it shouldn't be explicit
		camera.invView();

		Mat4f vp = camera.view * camera.project();

		Vec3f rX = X * vp;
		Vec3f rY = Y * vp;
		Vec3f rZ = Z * vp;
		Vec3f rOrigin = origin * vp;
		
		drawLine(rOrigin, rX, red);
		drawLine(rOrigin, rY, green);
		drawLine(rOrigin, rZ, blue);

		for (int i = 0; i < model.facesNumber(); i++) {
			Vec3f triVert[3];
			Vec3f rTriVert[3];
			Vec3f textureUV[3];

			Mat4f modelTransfrom = rotationX(anglePhi) * rotationY(angleTheta) * scale(scaleVariable / 2);
			for (int j = 0; j < 3; j++) {
				triVert[j] = model.triVert(i, j);
				textureUV[j] = model.triUV(i, j);
				triVert[j] = triVert[j] * modelTransfrom;
				rTriVert[j] = triVert[j] * vp;
				if (rTriVert[j].x < -1.0f || rTriVert[j].x > 1.0f || rTriVert[j].y < -1.0f || rTriVert[j].y > 1.0f) {
					continue;
				}
				viewport(rTriVert[j], render.imagebuffer.width, render.imagebuffer.height);
			}

			rasterize(rTriVert, triVert, render.models[0]->texture, textureUV);
		}

		render.imagebuffer.flip_vertically();
		Win32DrawToWindow(window, render.imagebuffer.data, render.imagebuffer.width, render.imagebuffer.height);

		char buffer[64];
		wsprintf(buffer, "%d ms\n", (int)fpsLock.milliElapsed());
		OutputDebugStringA(buffer);
	}

	return 0;
}