#pragma once

struct Color {
	union {
		struct {
			unsigned char r, g, b, a;
		};
		unsigned char rgba[4];
	};

	Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255) : r(r), g(g), b(b), a(a) {}

	void swapRBChannels() {
		unsigned char temp = r;
		r = b;
		b = temp;
	}
};


Color black(0, 0, 0);
Color white(255, 255, 255);
Color red(0, 0, 255);
Color green(0, 255, 0);
Color blue(255, 0, 0);
Color magenta(255, 0, 255);
Color peach(185, 218, 255);