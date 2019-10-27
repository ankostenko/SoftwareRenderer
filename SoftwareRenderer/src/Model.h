#pragma once

#include <vector>
#include "srmath.h"

struct Face {
	std::vector<int> vertices;
	std::vector<int> textureUV;
};

struct Model {
	std::vector<Face> faces;
	std::vector<Vec3f> vertices;
	std::vector<Vec3f> textureUV;

	Vec3f triVert(int iface, int nvert) {
		// TODO: I don't like the way this thing is written
		return vertices[faces[iface].vertices[nvert]];
	}

	Vec3f triUV(int iface, int nvert) {
		if (textureUV.empty()) return { 1, 1, 1 };
		return textureUV[faces[iface].textureUV[nvert]];
	}

	int facesNumber() {
		return faces.size();
	}
};