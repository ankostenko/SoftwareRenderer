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

int imageWidth = 1200;
int imageHeight = 600;

int main(int argc, char **argv) {
	initRenderer(imageWidth, imageHeight);

	loadModel(model, "models\\triking.obj"); 
	normalizeModelCoords(model);

	// TODO: default texture loading
	loadTexture(model, "models\\grid.jpg");

	clearImBuffer(black);

	HWND window = Win32CreateWindow(imageWidth, imageHeight, "3D Renderer");

	float angleAlpha = 0;
	float angleBeta = 0;
	float angleGamma = 0;
	float cameraAngleAlpha = 0;
	float cameraAngleBeta = 0;
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

		ProcessInput(window, angleAlpha, angleBeta, angleGamma, cameraAngleAlpha, cameraAngleBeta, scaleVariable, deltaTime);

		clearZBuffer(camera.farClippingPlane);
		clearImBuffer(peach);

		// Camera
		camera.position = camera.staticPosition * translate(0.0f, 0.0f, 2.0f) * rotate(cameraAngleAlpha, cameraAngleBeta, 0);
		camera.lookAt(origin);
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
			Vec3f normals[3];

			Mat4f modelTransform = translate(0.4f, -0.6f, -0.65f) * scale(scaleVariable) * scaleY(1.0f) * rotate(angleAlpha, angleBeta, angleGamma);
			for (int j = 0; j < 3; j++) {
				triVert[j] = model.triVert(i, j);
				textureUV[j] = model.triUV(i, j);
				normals[j] = model.triNorm(i, j);
				rTriVert[j] = triVert[j] * modelTransform * vp;
				normals[j] = normals[j] * inverse(transpose(modelTransform));
				if (rTriVert[j].x < -1.0f || rTriVert[j].x > 1.0f || rTriVert[j].y < -1.0f || rTriVert[j].y > 1.0f) {
					continue;
				}
				viewport(rTriVert[j], render.imagebuffer.width, render.imagebuffer.height);
			}
			rasterize(rTriVert, normals, render.models[0]->texture, textureUV);
		}
		render.imagebuffer.flip_vertically();
		Win32DrawToWindow(window, render.imagebuffer.data, render.imagebuffer.width, render.imagebuffer.height);

		char buffer[64];
		wsprintf(buffer, "%d ms - %d\n", (int)fpsLock.milliElapsed(), int(scaleVariable * 10));
		OutputDebugStringA(buffer);
	}

	return 0;
}