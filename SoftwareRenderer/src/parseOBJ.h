#pragma once
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include "srmath.h"
#include "Model.h"
#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../lib/stb_image.h"

Vec3i splitVertAndUv(std::string &str) {
	int vert;
	int vn = 0;
	int vt = 0;

	vert = std::stoi(str.substr(0, str.find('/')));
	vt = std::stoi(str.substr(str.find_last_of('/') + 1, std::string::npos));

	return { vert - 1, vn - 1, vt - 1 };
}

void loadModel(Model &model, const char *pathname) {
	std::ifstream file;
	file.open(pathname);

	if (!file) {
		std::cout << "Couldn't open the file" << std::endl;
		return;
	}
		
	std::string line, key, x, y, z, uvX, uvY, uvZ;
	char garbage;
	while (!file.eof()) {
		std::getline(file, line);
		std::istringstream iss(line);
		
		iss >> key;
		if (key == "v") {
			Vec3f temp;

			iss >> x >> y >> z;
			temp.x = std::stof(x);
			temp.y = std::stof(y);
			temp.z = std::stof(z);

			model.vertices.push_back(temp);
		} else if (key == "f") {
			Face face;

			iss >> x >> y >> z;

			Vec3i temp = splitVertAndUv(x);
			face.vertices.push_back(temp.x);
			face.textureUV.push_back(temp.z);
			temp = splitVertAndUv(y);
			face.vertices.push_back(temp.x);
			face.textureUV.push_back(temp.z);
			temp = splitVertAndUv(z);
			face.vertices.push_back(temp.x);
			face.textureUV.push_back(temp.z);

			model.faces.push_back(face);
		} else if (key == "vt") {
			Vec3f uv;

			iss >> uvX >> uvY;

			uv.x = std::stof(uvX);
			uv.y = std::stof(uvY);

			model.textureUV.push_back(uv);
		}
	}
 
	file.close();
}

void loadTexture(Image &tex, const char *pathname) {
	int width, height, nCh;

	unsigned char *data = stbi_load(pathname, &width, &height, &nCh, 0);

	tex.width = width;
	tex.height = height;
	tex.bytepp = nCh;
	tex.data = data;
	//stbi_image_free(data);
}