#include <vector>
#include <cassert>
#include "parseOBJ.h"
#include "Color.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../lib/stb_image_write.h"


Model model;

Color black(0, 0, 0);
Color white(255, 255, 255);
Color red(255, 0, 0);

const int imageHeight = 1600;
const int imageWidth = 1600;

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
		assert(x >= 0 && y >= 0 && x < width && y < height);
		memmove(&data[(x + y * width) * bytepp], color.rgba, bytepp);
	}
};

void drawLine(Vec3f vertex1, Vec3f vertex2, Color &color, Image &pixelBuffer) {
	//NDC to viewport transform
	//int x1 = (vertex1.x + 1) * pixelBuffer.width * 0.5;
	//int y1 = (-vertex1.y + 1) * pixelBuffer.height * 0.5;
	//int x2 = (vertex2.x + 1) * pixelBuffer.width  * 0.5;
	//int y2 = (-vertex2.y + 1) * pixelBuffer.height * 0.5;

	int x1 = vertex1.x;
	int y1 = vertex1.y;
	int x2 = vertex2.x;
	int y2 = vertex2.y;

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
			pixelBuffer.set(y, x,  color); //Swap back because of steep transpose
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

int main() {
	
	// Parse obj
	parseOBJ(model, ".\\models\\deer.obj");

	Image image(imageWidth, imageHeight);

	for (int y = 0; y < imageHeight; y++) {
		for (int x = 0; x < imageWidth; x++) {
			image.set(x, y, black);
		}
	}


	for (int i = 0; i < model.facesNumber; i++) {
		Vec3f triVert[3];
		for (int j = 0; j < 3; j++) {
			triVert[j] = model.triVert(i, j);
		}
		drawLine(triVert[0], triVert[1], red, image);
		drawLine(triVert[1], triVert[2], red, image);
		drawLine(triVert[2], triVert[0], red, image);
	}

	stbi_write_png("output.png", imageWidth, imageHeight, image.bytepp, image.data, imageWidth * image.bytepp);
}