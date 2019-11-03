#include "internal.h"

#include "srmath.h"
#include "color.h"
#include "image.h"
#include "model.h"

#include "winlayer.h"
#include "timer.cpp"

#include "renderer.h"
#include "renderer.cpp"

#include "load.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../lib/stb_image_write.h"


bool globalRunning = true;
bool globalPause = false;
Model model;

int imageWidth =  1600;
int imageHeight = 900;

// Camera features 
Vec3f camera = { 0.0f, 0.0f, 0.0f };
float nearClippingPlane = 0.1f;
float farClippingPlane = 1000.0f;
float fov = (float)M_PI / 3;

int main(int argc, char **argv) {
	initRenderer(imageWidth, imageHeight);

	loadModel(model, "models\\new_african_head.obj");
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

	Mat4f proj = projection(fov, render.imagebuffer.width / (float)render.imagebuffer.height, nearClippingPlane, farClippingPlane);

	while (globalRunning) {
		if (fpsLock.milliElapsed() > 16.0f) {
			fpsLock.ResetStartTime();
			
			ProcessInput(window, angleTheta, anglePhi, cameraAngleTheta, cameraAnglePhi, scaleVariable);

			clearZBuffer(farClippingPlane);
			clearImBuffer(peach);

			// Camera
			Vec3f tCamera = camera * translate(0.0f, 0.0f, 1.5f) * rotationX(cameraAnglePhi) * rotationY(cameraAngleTheta);
			Mat4f view = lookAt(tCamera, origin);
			view = inverse(view);

			Vec3f rX = X * view * proj;
			Vec3f rY = Y * view * proj;
			Vec3f rZ = Z * view * proj;
			Vec3f rOrigin = origin * view * proj;

			drawLine(rOrigin, rX, render.zbuffer, red);
			drawLine(rOrigin, rY, render.zbuffer, green);
			drawLine(rOrigin, rZ, render.zbuffer, blue);

			for (int i = 0; i < model.facesNumber(); i++) {
				Vec3f triVert[3];
				Vec3f rTriVert[3];
				Vec3f textureUV[3];
				for (int j = 0; j < 3; j++) {
					triVert[j] = model.triVert(i, j);
					textureUV[j] = model.triUV(i, j);
					triVert[j] = triVert[j] * scale(scaleVariable / 2) * rotationXY(angleTheta, anglePhi);
					rTriVert[j] = triVert[j] * view * proj;
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
	}

	return 0;
}