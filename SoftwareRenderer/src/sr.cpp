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

Model model;

Color black(0, 0, 0);
Color white(255, 255, 255);
Color red(0, 0, 255);
Color green(0, 255, 0);
Color blue(255, 0, 0);
Color magenta(255, 0, 255);

const int imageHeight = 800;
const int imageWidth = 800;

// Camera features 

Vec3f camera = { 3.0f, 3.0f, 3.0f };
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

bool computePixelCoordinates(Mat4f view, Vec3f pWorld, float b, float l, float r, float t, int imageWidth, int imageHeight, Vec3f &pRaster) {
	Vec3f pCamera = pWorld * view;
	
	Vec3f pScreen;
	if (pCamera.z < 1e-5) return false;
	pScreen.x = pCamera.x / -pCamera.z * nearClippingPlane;
	pScreen.y = pCamera.y / -pCamera.z * nearClippingPlane;

	Vec3f pNDC;
	pNDC.x = (pScreen.x + r) / (2 * r);
	pNDC.y = (pScreen.y + t) / (2 * t);

	pRaster.x = (int)(pNDC.x * imageWidth);
	pRaster.y = (int)((1 - pNDC.y) * imageHeight);

	if (pScreen.x < l || pScreen.x > r || pScreen.y < b || pScreen.y > t) {
		return false;
	}
	return true;
}

void drawLine(Vec3f p1, Vec3f p2, Image &buffer, Color &color) {
	float x1 = p1.x;
	float y1 = p1.y;
	float x2 = p2.x;
	float y2 = p2.y;
	
	//transpose line if it is too steep
	bool steep = false;
	if (std::abs(x1 - x2) < std::abs(y1 - y2)) {
		std::swap(x1, y1);
		std::swap(x2, y2);
		steep = true;
	}

	//Redefine line so that it is left to right
	if (x1 > x2) {
		std::swap(x1, x2);
		std::swap(y1, y2);
	}

	//Redefined to use only int arithmetic
	int dx = x2 - x1;
	int dy = y2 - y1;
	int derror2 = std::abs(dy) * 2;
	int error2 = 0;
	int y = y1;

	for (int x = x1; x <= x2; x++) {
		if (steep) {
			buffer.set(y, x, color); //Swap back because of steep transpose
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
	ndc.y = (pScreen.y + top)   / (2 * top  );

	coords.x = (int)(ndc.x * imageWidth);
	coords.y = (int)((1.0f - ndc.y) * imageHeight);

	return true;
}

int main(int argc, char **argv) {
	parseOBJ(model, ".\\models\\teapot.obj");

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

	Timer fpsLock;

	float filmAspectRatio = filmApertureWidth / filmApertureHeight;
	float deviceAspectRatio = imageWidth / (float)imageHeight;

	float top = ((filmApertureHeight * inch2mm / 2) / focalLength) * nearClippingPlane;
	float right = ((filmApertureWidth * inch2mm / 2) / focalLength) * nearClippingPlane;

	float xscale = 1;
	float yscale = 1;

	if (filmAspectRatio > deviceAspectRatio) {
		xscale = deviceAspectRatio / filmAspectRatio;
	} else {
		yscale = filmAspectRatio / deviceAspectRatio;
	}

	right *= xscale;
	top *= yscale;

	float left = -right;
	float bottom = -top;

	// World coordinate system
	Vec3f X = { 1.0f, 0.0f, 0.0f };
	Vec3f Y = { 0.0f, 1.0f, 0.0f };
	Vec3f Z = { 0.0f, 0.0f, 1.0f };
	Vec3f origin = { 0.0f, 0.0f, 0.0f };

	while (1) {
		if (fpsLock.milliElapsed() > 16.0f) {
			fpsLock.ResetStartTime();

			memset(image.data, 0, image.size());

			ProcessInput(window, angleTheta, anglePhi, cameraAngleTheta, cameraAnglePhi);
			
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
					triVert[j] = triVert[j] * scale(0.4) * rotationXY(angleTheta, anglePhi);
					pixelCoord(view, triVert[j], right, top, imageWidth, imageHeight, rTriVert[j]);
				}

				drawLine(rTriVert[0], rTriVert[1], image, white);
				drawLine(rTriVert[1], rTriVert[2], image, white);
				drawLine(rTriVert[2], rTriVert[0], image, white);
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
	stbi_write_png("output.png", imageWidth, imageHeight, image.bytepp, image.data, imageWidth * image.bytepp);

	//system("pause");

	//SDL_Quit();
	return 0;
}