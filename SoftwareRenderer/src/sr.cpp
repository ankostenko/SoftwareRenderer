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

Vec3f camera = { -1.0f, 1.0f, -3.0f };
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

void drawLine(Vec3f vertex1, Vec3f vertex2, Color &color, Image &pixelBuffer) {
	// To NDC
	// constant is expressing how far from camera Z-plane is located
	const int HOW_FAR_Z_PLANE = 5;
	//	2.0 and 4.0 are a normalization constants
	float x1 = vertex1.x / ((vertex1.z + 2.0f) / 4 + HOW_FAR_Z_PLANE);
	float y1 = -vertex1.y / ((vertex1.z + 2.0f) / 4 + HOW_FAR_Z_PLANE);
	float x2 = vertex2.x / ((vertex2.z + 2.0f) / 4 + HOW_FAR_Z_PLANE);
	float y2 = -vertex2.y / ((vertex2.z + 2.0f) / 4 + HOW_FAR_Z_PLANE);

	// NDC to viewport
	x1 = (x1 + 1.0f) / 2 * pixelBuffer.width;
	x2 = (x2 + 1.0f) / 2 * pixelBuffer.width;
	y1 = (y1 + 1.0f) / 2 * pixelBuffer.height + pixelBuffer.height / 4;
	y2 = (y2 + 1.0f) / 2 * pixelBuffer.height + pixelBuffer.height / 4;

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
			pixelBuffer.set(y, x, color); //Swap back because of steep transpose
		}
		else {
			pixelBuffer.set(x, y, color);
		}
		error2 += derror2;
		if (error2 > dx) {
			y += (y2 > y1 ? 1 : -1);
			error2 -= dx * 2;
		}
	}
}

bool computePixelCoordinates(Mat4f &c2w, float b, float l, float r, float t, int imageWidth, int imageHeight) {
	Vec3f pCamera = camera * c2w;

	Vec3f pScreen;
	pScreen.x = pCamera.x / pCamera.z * nearClippingPlane;
	pScreen.y = pCamera.y / pCamera.z * nearClippingPlane;

	Vec3f pNDC;
	pNDC.x = (pScreen.x + r) / (2 * r);
	pNDC.y = (pScreen.y + t) / (2 * t);

	Vec3f pRaster;
	pRaster.x = (int)(pNDC.x * imageWidth);
	pRaster.y = (int)(pNDC.y * imageHeight);

	if (pScreen.x < l || pScreen.x > r || pScreen.y < b || pScreen.y > t) {
		return false;
	}
	return true;
}

int main(int argc, char **argv) {
	Mat4f scaleMatrix = scale(3.75f);
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

	Timer fpsLock;

	float right = ((filmApertureWidth * inch2mm / 2) / focalLength) * nearClippingPlane;
	float left = -right;

	float top = ((filmApertureHeight * inch2mm / 2) / focalLength) * nearClippingPlane;
	float bottom = -top;

	Mat4f cameraTransform = identity();
	computePixelCoordinates(cameraTransform, bottom, left, right, top, image.width, imageHeight);
		 
	while (1) {
		if (fpsLock.milliElapsed() > 16.0f) {
			fpsLock.ResetStartTime();

			memset(image.data, 0, image.size());

			ProcessInput(window, angleTheta, anglePhi);

			// anglePhi += M_PI / (20 * M_PI);

			//Mat4f transform = rotationX(M_PI / 4);


			// World coordinate system
			Vec3f x = { 3.0f, 0.0f, 0.0f };
			Vec3f y = { 0.0f, 3.0f, 0.0f };
			Vec3f z = { 0.0f, 0.0f, 3.0f };
			Vec3f origin = { 0.0f, 0.0f, 0.0f };
			// Camera
			Mat4f view = lookAt(camera, origin);

			x = x * view;
			y = y * view;
			z = z * view;

			drawLine(origin, x, red, image);
			drawLine(origin, y, green, image);
			drawLine(origin, z, blue, image);
			// ===================================

			// DEBUG: ARROW POINTING TO UP
			Vec3f localUp = { 0.0f, 5.0f, 0.0f };
			Vec3f arrow[] = { { 0.1f, 4.9f, 0.1f }, { -0.1f, 4.9f, 0.1f }, { 0.0f, 4.9f, -0.1f } };
			arrow[0] = arrow[0] * rotationXY(angleTheta, anglePhi);
			arrow[1] = arrow[1] * rotationXY(angleTheta, anglePhi);
			arrow[2] = arrow[2] * rotationXY(angleTheta, anglePhi);
			localUp = localUp * rotationXY(angleTheta, anglePhi);
			drawLine(origin, localUp, magenta, image);
			drawLine(arrow[0], localUp, magenta, image);
			drawLine(arrow[1], localUp, magenta, image);
			drawLine(arrow[2], localUp, magenta, image);
			// ======================

			for (int i = 0; i < model.facesNumber(); i++) {
				Vec3f triVert[3];
				for (int j = 0; j < 3; j++) {
					triVert[j] = model.triVert(i, j);
					triVert[j] = triVert[j] * rotationXY(angleTheta, anglePhi) * view;
					//triVert[j] = triVert[j] * rotationX(anglePhi) * rotationY(angleTheta); // It is probably a model rotation
					//triVert[j] = triVert[j] * rotationY(angleTheta) * rotationX(anglePhi); // It is probably a model rotation
				}

				drawLine(triVert[0], triVert[1], white, image);
				drawLine(triVert[1], triVert[2], white, image);
				drawLine(triVert[2], triVert[0], white, image);
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