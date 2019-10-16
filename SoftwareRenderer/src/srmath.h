#pragma once

struct Vec3f {
	union {
		struct {
			float x, y, z;
		};
		float r[3];
	};
};