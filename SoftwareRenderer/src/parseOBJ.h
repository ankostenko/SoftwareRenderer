#pragma once
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include "srmath.h"
#include "Model.h"

Vec3f splitVertAndUv(std::string &str) {
	float vert;
	float vn = 0;
	float vt = 0;

	vert = std::stof(str.substr(0, str.find('/')));
	vt = std::stof(str.substr(str.find_last_of('/') + 1, std::string::npos));

	return { vert, vn, vt };
}

void parseOBJ(Model &model, const char *pathname) {
	std::ifstream file;
	file.open(pathname);

	if (!file) {
		std::cout << "Couldn't open the file" << std::endl;
		return;
	}
		
	std::string line, key, x, y, z;
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
			iss >> x >> y >> z;
			std::vector<Vec3f> f;

			f.push_back(splitVertAndUv(x));
			f.push_back(splitVertAndUv(y));
			f.push_back(splitVertAndUv(z));
			
			model.faces.push_back(f);
		}
	}
 
	file.close();
}