#pragma once

#include <vector>
#include "srmath.h"

struct Model {
	std::vector<Vec3f> vertices;
	std::vector<std::vector<Vec3f>> faces;
	std::vector<Vec3f> textureUV;

	Vec3f triVert(int iface, int nvert) {
		return vertices[faces[iface][nvert]];
	}
};