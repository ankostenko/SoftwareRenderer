#pragma once

struct Color {
	union {
		struct {
			unsigned char r, g, b, a;
		};
		unsigned char rgba[4];
	};

	Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255) : r(r), g(g), b(b), a(a) {}
};