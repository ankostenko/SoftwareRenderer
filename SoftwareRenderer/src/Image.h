#pragma once

struct Image {
public:
	enum BytePerPixel {
		MONO = 1, RGB = 3, RGBA = 4,
	};

public:
	unsigned char *data;
	int width, height;
	int bytepp;
	int aaCoeff;

	Image() { }

	Image(int width, int height, int aaCoeff, BytePerPixel bytepp = RGB) : width(width), height(height), aaCoeff(aaCoeff), bytepp(bytepp) {
		data = new unsigned char[bytepp * aaCoeff * width * height];
	}

	void set(int x, int y, Color color) {
		if (x < 0 || y < 0 || x >= width || y >= height) {
			return;
		}

		memmove(&data[(x + y * width) * bytepp], color.rgba, bytepp);
	}

	void set(int pxIdx, Color color) {
		if (pxIdx < 0 || pxIdx >= width * height * aaCoeff) {
			return;
		}
		
		memmove(&data[pxIdx * bytepp], color.rgba, bytepp);
	}

	Color get(float u, float v) {
		Color color(0, 0, 0);

		int x = int(roundf(u * width) );
		int y = int(roundf(v * height));

		if (x < 0 || y < 0 || x >= width || y >= height) {
			return color;
		}

		memmove(color.rgba, &data[(x + y * width) * bytepp], bytepp);
		
		return color;
	}

	Color get(int pxIdx) {
		Color color(0, 0, 0);

		memmove(color.rgba, &data[pxIdx * bytepp], bytepp);
		return color;
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

struct Texture {
	Image texture;
};