#pragma once

struct Face {
	int  vertices[3];
	int textureUV[3];
	int normals[3];
};

struct Model {
	std::vector<Face> faces;
	std::vector<Vec3f> vertices;
	std::vector<Vec3f> textureUV;
	std::vector<Vec3f> normals;
	Texture texture;

	Vec3f triVert(int iface, int nthvert) {
		// TODO: I don't like the way this thing is written
		return vertices[faces[iface].vertices[nthvert]];
	}

	Vec3f triUV(int iface, int nthvert) {
		if (textureUV.empty()) return Vec3f({ 1, 1, 1 });
		return textureUV[faces[iface].textureUV[nthvert]];
	}

	Vec3f triNorm(int iface, int nthvert) {
		return normals[faces[iface].normals[nthvert]];
	}

	size_t facesNumber() {
		return faces.size();
	}
};