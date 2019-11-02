#include <vector>
#include <cassert>
#define _USE_MATH_DEFINES
#include <math.h>
#include "srmath.h"
#include "parseOBJ.h"
#include "Color.h"
#include "Image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../lib/stb_image_write.h"

#include "winlayer.h"
#include "Timer.cpp"

bool globalRunning = true;
bool globalPause = false;
Model model;
Image texture;

Vec3f lightDir = { -2.0f, -2.0f, -4.0f };

Color black(0, 0, 0);
Color white(255, 255, 255);
Color red(0, 0, 255);
Color green(0, 255, 0);
Color blue(255, 0, 0);
Color magenta(255, 0, 255);
Color peach(185, 218, 255);

int imageWidth =  1600;
int imageHeight = 900;

// Camera features 

Vec3f camera = { 0.0f, 0.0f, 0.0f };
float nearClippingPlane = 0.1f;
float farClippingPlane = 1000.0f;
float fov = M_PI / 3;

void drawLine(Vec3f p1, Vec3f p2, float *zbuffer, Image &buffer, Color &color) {
	float x1 = p1.x;
	float y1 = p1.y;
	float x2 = p2.x;
	float y2 = p2.y;
	p1.z = 1 / p1.z;
	p2.z = 1 / p2.z;

	bool steep = false;
	if (std::abs(x1 - x2) < std::abs(y1 - y2)) {
		std::swap(x1, y1);
		std::swap(x2, y2);
		steep = true;
	}

	if (x1 > x2) {
		std::swap(x1, x2);
		std::swap(y1, y2);
		std::swap(p1.z, p2.z);
	}

	int dx = x2 - x1;
	int dy = y2 - y1;
	int derror2 = std::abs(dy) * 2;
	int error2 = 0;
	int y = y1;

	for (int x = x1; x <= x2; x++) {
		float t = (x - x1) / (x2 - x1);
		float z = 1 / ((1 - t) * (p1.z) + t * (p2.z));
		if (steep) {
			if (x >= 0 && y >= 0 && y < buffer.width && x < buffer.height) {
				if (z < zbuffer[y + x * buffer.width]) {
					zbuffer[y + x * buffer.width] = z;
				}
			}
			buffer.set(y, x, color);
		}
		else {
			if (x >= 0 && y >= 0 && x < buffer.width && y < buffer.height) {
				if (z < zbuffer[x + y * buffer.width]) {
					zbuffer[x + y * buffer.width] = z;
				}
			}
			buffer.set(x, y, color);
		}
		error2 += derror2;
		if (error2 > dx) {
			y += (y2 > y1 ? 1 : -1);
			error2 -= dx * 2;
		}
	}
}

bool pixelCoord(Mat4f &view, Vec3f pWorld, float right, float top, int imageWidth, int imageHeight, Vec3f &coords) {
	Vec3f pCamera = pWorld * view;

	Vec3f pScreen;
	pScreen.x = pCamera.x / -pCamera.z * nearClippingPlane;
	pScreen.y = pCamera.y / -pCamera.z * nearClippingPlane;

	float left = -right;
	float bottom = -top;
	
	if (pScreen.x > right) {
		pScreen.x = right;
	}
	if (pScreen.x < left) {
		pScreen.x = left;
	}
	if (pScreen.y < bottom) {
		pScreen.y = bottom;
	}
	if (pScreen.y > top) {
		pScreen.y = top;
	}

	Vec3f ndc;
	ndc.x = (pScreen.x + right) / (2 * right);
	ndc.y = (pScreen.y + top) / (2 * top);

	coords.x = int((ndc.x * imageWidth));
	coords.y = int(((1.0f - ndc.y) * imageHeight));
	coords.z = -pCamera.z;

	return true;
}

float edgeFunction(const Vec3f &a, const Vec3f &b, const Vec3f &c) {
	return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
}

#define DEBUG_HAS_TEXTURE 0
void rasterize(Vec3f *triVert, Vec3f *globalVert, Vec3f *uv, Image &imagebuffer, float *zbuffer) {
	triVert[0].z = 1 / triVert[0].z;
	triVert[1].z = 1 / triVert[1].z;
	triVert[2].z = 1 / triVert[2].z;

#if DEBUG_HAS_TEXTURE
	uv[0] = uv[0] * triVert[0].z;
	uv[1] = uv[1] * triVert[1].z;
	uv[2] = uv[2] * triVert[2].z;
#endif
	// Find "border" box
	int minX = triVert[0].x;
	int minY = triVert[0].y;
	int maxX = triVert[0].x;
	int maxY = triVert[0].y;

	for (int i = 1; i < 3; i++) {
		if (triVert[i].x < minX) {
			minX = triVert[i].x;
		}
		if (triVert[i].x > maxX) {
			maxX = triVert[i].x;
		}
		if (triVert[i].y < minY) {
			minY = triVert[i].y;
		}
		if (triVert[i].y > maxY) {
			maxY = triVert[i].y;
		}
	}

	Vec3f edgeNormal = norm(cross(globalVert[2] - globalVert[0], globalVert[1] - globalVert[0]));
	float intensity = norm(lightDir) * edgeNormal;
	if (intensity > 1.0f) {
		intensity = 1.0f;
	}
	if (intensity < 0) {
		intensity = -intensity * 0.3;
	}

	float area = edgeFunction(triVert[0], triVert[1], triVert[2]);
	if (area <= 0) {
		return;
	}

	for (int y = minY; y <= maxY; y++) {
		for (int x = minX; x <= maxX; x++) {
			Vec3f p = { x, y, 0 };
			// Determine whether point inside the triangle or not
			float w0 = edgeFunction(triVert[1], triVert[2], p);
			float w1 = edgeFunction(triVert[2], triVert[0], p);
			float w2 = edgeFunction(triVert[0], triVert[1], p);

			if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
				// Barycentric coordinates
				w0 /= area;
				w1 /= area;
				w2 /= area;
				
				float z = 1 / (triVert[0].z * w0 + triVert[1].z * w1 + triVert[2].z * w2);

				// Clipping
				if (x >= imagebuffer.width || x < 0 || y >= imagebuffer.height || y < 0) {
					continue; 
				}

				if (z < zbuffer[x + y * imagebuffer.width]) {
					zbuffer[x + y * imagebuffer.width] = z;
					// TODO: Investigate what the hell is going on with texture mapping
					// Texture mapping is working only after transformations done by MeshLab
					// It outputs completely different mapping and it does something but I'm not sure what
#if DEBUG_HAS_TEXTURE
					Vec3f uvC = uv[0] * w0 + uv[1] * w1 + uv[2] * w2;
					uvC = uvC * z;
					Color color = texture.get(uvC.x, uvC.y);
					
					color.swapRBChannels();
					color.r *= intensity;
					color.g *= intensity;
					color.b *= intensity;
#else
					Color color(white.r * intensity, white.g * intensity, white.b * intensity);
#endif
					imagebuffer.set(x, y, color);	
				}

			}
		}
	}
}

int main(int argc, char **argv) {
	loadModel(model, "models\\african_head.obj");
	normalizeModelCoords(model);

	loadTexture(texture, "models\\african_head_diffuse.jpg");
	texture.flip_vertically();

	Image image(imageWidth, imageHeight);

	for (int y = 0; y < imageHeight; y++) {
		for (int x = 0; x < imageWidth; x++) {
			image.set(x, y, black);
		}
	}

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

	float *zbuffer = new float[image.width * image.height];

	Mat4f proj = projection(fov, image.width / (float)image.height, nearClippingPlane, farClippingPlane);

	while (globalRunning) {
		if (fpsLock.milliElapsed() > 16.0f) {
			fpsLock.ResetStartTime();
			
			ProcessInput(window, angleTheta, anglePhi, cameraAngleTheta, cameraAnglePhi, scaleVariable);

			for (int i = 0; i < image.width * image.height; i++) {
				zbuffer[i] = farClippingPlane;
			}

			// memset(image.data, 0, image.size());
			for (int i = 0; i < image.size(); i += image.bytepp) {
				memmove(&image.data[i], peach.rgba, image.bytepp);
			}

			// Camera
			Vec3f tCamera = camera * translate(0.0f, 0.0f, 1.5f) * rotationX(cameraAnglePhi) * rotationY(cameraAngleTheta);
			Mat4f view = lookAt(tCamera, origin);
			view = inverse(view);

			Vec3f rX = X * view * proj;
			Vec3f rY = Y * view * proj;
			Vec3f rZ = Z * view * proj;
			Vec3f rOrigin = origin * view * proj;

			// Viewport transform
			viewport(rX, image.width, image.height);
			viewport(rY, image.width, image.height);
			viewport(rZ, image.width, image.height);
			viewport(rOrigin, image.width, image.height);
			
			drawLine(rOrigin, rX, zbuffer, image, red);
			drawLine(rOrigin, rY, zbuffer, image, green);
			drawLine(rOrigin, rZ, zbuffer, image, blue);

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
					viewport(rTriVert[j], image.width, image.height);
				}
				
				rasterize(rTriVert, triVert, textureUV, image, zbuffer);
			}

			Win32DrawToWindow(window, image);
			
			char buffer[64];
			wsprintf(buffer, "%d ms\n", (int)fpsLock.milliElapsed());
			OutputDebugStringA(buffer);
		}
	}

	return 0;
}