#pragma once

struct Vec3f {
	union {
		struct {
			float x, y, z;
		};
		float r[3];
	};

	// If it doesn't have any cost I let it be
	float operator[](int i) {
		return r[i];
	}
};