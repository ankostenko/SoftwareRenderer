#include <vector>
#include <cassert>
#define _USE_MATH_DEFINES
#include <math.h>
#include "parseOBJ.h"
#include "Color.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../lib/stb_image_write.h"

#include "winlayer.h"
#include "Timer.cpp"

bool globalRunning = true;
Model model;

Vec3f lightDir = { 2.0f, 1.0f, -4.0f };

Color black(0, 0, 0);
Color white(255, 255, 255);
Color red(0, 0, 255);
Color green(0, 255, 0);
Color blue(255, 0, 0);
Color magenta(255, 0, 255);
Color peach(185, 218, 255);

int imageWidth =  1300;
int imageHeight = 900;

// Camera features 

Vec3f camera = { 0.0f, 3.0f, 3.0f };
float focalLength = 35;
float filmApertureWidth = 0.980;
float filmApertureHeight = 0.735;
const float inch2mm = 25.4f;
float nearClippingPlane = 0.1f;
float farClippingPlane = 1000.0f;

struct Image {
private:
	enum BytePerPixel {
		MONO = 1, RGB = 3, RGBA = 4,
	};

public:
	unsigned char *data;
	int width, height;
	int bytepp;

	Image(int width, int height, BytePerPixel bytepp = RGB) : width(width), height(height), bytepp(bytepp) {
		data = new unsigned char[bytepp * width * height];
	}

	void set(int x, int y, Color color) {
		if (x < 0 || y < 0 || x >= width || y >= height) {
			return;
		}

		memmove(&data[(x + y * width) * bytepp], color.rgba, bytepp);
	}

	size_t size() {
		return width * height * bytepp;
	}

	void flip_vertically() {
		void *temp = new unsigned char[width * bytepp];
		for (int y = 0; y < height / 2; y++) {
			memcpy(temp, &data[y * width * bytepp], width * bytepp);
			memcpy(&data[y * width * bytepp], &data[(height - 1 - y) * width * bytepp], width * bytepp);
			memcpy(&data[(height - 1 - y) * width * bytepp], temp, width * bytepp);
		}
		delete temp;
	}
};

void drawLine(Vec3f p1, Vec3f p2, Image &buffer, Color &color) {
	float x1 = p1.x;
	float y1 = p1.y;
	float x2 = p2.x;
	float y2 = p2.y;

	bool steep = false;
	if (std::abs(x1 - x2) < std::abs(y1 - y2)) {
		std::swap(x1, y1);
		std::swap(x2, y2);
		steep = true;
	}

	if (x1 > x2) {
		std::swap(x1, x2);
		std::swap(y1, y2);
	}

	int dx = x2 - x1;
	int dy = y2 - y1;
	int derror2 = std::abs(dy) * 2;
	int error2 = 0;
	int y = y1;

	for (int x = x1; x <= x2; x++) {
		if (steep) {
			buffer.set(y, x, color);
		}
		else {
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

	if (pScreen.x > right || pScreen.x < left || pScreen.y > top || pScreen.y < bottom) {
		return false;
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
	Vec3i ai = { int(a.x), int(a.y), int(a.z) };
	Vec3i bi = { int(b.x), int(b.y), int(b.z) };
	Vec3i ci = { int(c.x), int(c.y), int(c.z) };
	return (ci.x - ai.x) * (bi.y - ai.y) - (ci.y - ai.y) * (bi.x - ai.x);
}

Vec3f barycentric(Vec3i v0, Vec3i v1, Vec3i v2, Vec3i p) {
	Vec3f bar = { };
	return bar;
}

void rasterize(Vec3f *triVert, Vec3f *globalVert, Image &imagebuffer, float *zbuffer) {
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
		intensity = 0;
	}

	Vec3f p;
	float area = edgeFunction(triVert[0], triVert[1], triVert[2]);
	if (area <= 0) {
		return;
	}

	for (p.y = minY; p.y < maxY; p.y++) {
		for (p.x = minX; p.x < maxX; p.x++) {
			// Determine whether point inside the triangle or not
			float w0 = edgeFunction(p, triVert[1], triVert[2]);
			float w1 = edgeFunction(p, triVert[2], triVert[0]);
			float w2 = edgeFunction(p, triVert[0], triVert[1]);
			
			if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
				// Barycentric coordinates
				w0 /= area;
				w1 /= area;
				w2 /= area;

				w0 = w0 + 10e-5;
				w1 = w1 + 10e-5;
				w2 = w2 + 10e-5;

				int x = (p.x * w0) + (p.x * w1) + (p.x * w2);
				int y = (p.y * w0) + (p.y * w1) + (p.y * w2);
				float z = 1 / (w0 / triVert[0].z + w1 / triVert[1].z + w2 / triVert[2].z);

				if (z < zbuffer[x + y * imagebuffer.width]) {
					zbuffer[x + y * imagebuffer.width] = z;
					Color color(255 * intensity, 255 * intensity, 255 * intensity);
					imagebuffer.set(x, y, color);
				}

			}
		}
	}
}

int main(int argc, char **argv) {
	loadModel(model, ".\\models\\teapot.obj");

	Image image(imageWidth, imageHeight);

	for (int y = 0; y < imageHeight; y++) {
		for (int x = 0; x < imageWidth; x++) {
			image.set(x, y, black);
		}
	}

	HWND window = Win32Init(imageWidth, imageHeight);
	HDC hdc = GetDC(window);

	float angleTheta = 0;
	float anglePhi = 0;
	float cameraAngleTheta = 0;
	float cameraAnglePhi = 0;
	float scaleVariable = 0.5f;

	Timer fpsLock;

	float filmAspectRatio = filmApertureWidth / filmApertureHeight;
	float deviceAspectRatio = imageWidth / (float)imageHeight;

	float top = ((filmApertureHeight * inch2mm / 2) / focalLength) * nearClippingPlane;
	float right = ((filmApertureWidth * inch2mm / 2) / focalLength) * nearClippingPlane;

	float xscale = 1;
	float yscale = 1;

#if 1
	if (filmAspectRatio > deviceAspectRatio) {
		xscale = deviceAspectRatio / filmAspectRatio;
	}
	else {
		yscale = filmAspectRatio / deviceAspectRatio;
	}
#else
	if (filmAspectRatio > deviceAspectRatio) {
		yscale = filmAspectRatio / deviceAspectRatio;
	}
	else {
		xscale = deviceAspectRatio / filmAspectRatio;
	}
#endif

	right *= xscale;
	top *= yscale;

	float left = -right;
	float bottom = -top;

	// World coordinate system
	Vec3f X = { 1.0f, 0.0f, 0.0f };
	Vec3f Y = { 0.0f, 1.0f, 0.0f };
	Vec3f Z = { 0.0f, 0.0f, 1.0f };
	Vec3f origin = { 0.0f, 0.0f, 0.0f };

	float *zbuffer = new float[image.width * image.height];

	while (globalRunning) {
		if (fpsLock.milliElapsed() > 16.0f) {
			fpsLock.ResetStartTime();

			for (int i = 0; i < image.width * image.height; i++) {
				zbuffer[i] = farClippingPlane;
			}

			// memset(image.data, 0, image.size());
			for (int i = 0; i < image.size(); i += image.bytepp) {
				memmove(&image.data[i], peach.rgba, image.bytepp);
			}

			ProcessInput(window, angleTheta, anglePhi, cameraAngleTheta, cameraAnglePhi, scaleVariable);

			// Camera
			Vec3f tCamera = camera * rotationX(cameraAnglePhi) * rotationY(cameraAngleTheta);
			Mat4f view = lookAt(tCamera, origin);
			view = inverse(view);

			Vec3f rX;
			Vec3f rY;
			Vec3f rZ;
			Vec3f rOrigin;

			if (!pixelCoord(view, X, right, top, imageWidth, imageHeight, rX)) continue;
			if (!pixelCoord(view, Y, right, top, imageWidth, imageHeight, rY)) continue;
			if (!pixelCoord(view, Z, right, top, imageWidth, imageHeight, rZ)) continue;
			if (!pixelCoord(view, origin, right, top, imageWidth, imageHeight, rOrigin)) continue;

			drawLine(rOrigin, rX, image, red);
			drawLine(rOrigin, rY, image, green);
			drawLine(rOrigin, rZ, image, blue);

			for (int i = 0; i < model.facesNumber(); i++) {
				Vec3f triVert[3];
				Vec3f rTriVert[3];
				for (int j = 0; j < 3; j++) {
					triVert[j] = model.triVert(i, j);
					triVert[j] = triVert[j] * scale(scaleVariable) * rotationXY(angleTheta, anglePhi);
				}
				if (!pixelCoord(view, triVert[0], right, top, imageWidth, imageHeight, rTriVert[0])) continue;
				if (!pixelCoord(view, triVert[1], right, top, imageWidth, imageHeight, rTriVert[1])) continue;
				if (!pixelCoord(view, triVert[2], right, top, imageWidth, imageHeight, rTriVert[2])) continue;

				rasterize(rTriVert, triVert, image, zbuffer);
			}

			image.flip_vertically();
			buffer_memory = image.data;
			StretchDIBits(hdc, 0, 0, imageWidth, imageHeight, 0, 0, imageWidth, imageHeight, buffer_memory, &buffer_bitmapinfo, DIB_RGB_COLORS, SRCCOPY);

			char buffer[64];

			wsprintf(buffer, "%d ms\n", (int)fpsLock.milliElapsed());
			OutputDebugStringA(buffer);

		}
	}

	// image.flip_vertically();
	// stbi_write_png("output.png", imageWidth, imageHeight, image.bytepp, image.data, imageWidth * image.bytepp);

	//system("pause");

	return 0;
}