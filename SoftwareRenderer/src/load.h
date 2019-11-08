#pragma once

Vec3i splitVertAndUv(std::string &str) {
	int vert;
	int vn = 0;
	int vt = 0;

	vert = std::stoi(str.substr(0, str.find('/')));
	if (str.find_last_of('/') - str.find('/') == 1) {
		vn = std::stoi(str.substr(str.find_last_of('/') + 1, std::string::npos));
	} else {
		vt = std::stoi(str.substr(str.find('/') + 1, str.find_last_of('/')));
		vn = std::stoi(str.substr(str.find_last_of('/') + 1, std::string::npos));
	}

	return { vert - 1, vn - 1, vt - 1 };
}

void normalizeModelCoords(Model &model) {
	float length = 0;
	for (int i = 0; i < model.vertices.size(); i++) {
		Vec3f vert = model.vertices.data()[i];

		float newLength = sqrt(vert.x * vert.x + vert.y * vert.y + vert.z * vert.z);
		if (newLength > length) {
			length = newLength;
		}
	}

	for (int i = 0; i < model.vertices.size(); i++) {
		Vec3f *vert = &model.vertices.data()[i];
		vert->x /= length;
		vert->y /= length;
		vert->z /= length;
	}
}

void loadModel(Model &model, const char *pathname) {
	std::ifstream file;
	file.open(pathname);

	Timer timer;
	if (!file) {
		std::cout << "Couldn't open the file" << std::endl;
		return;
	}
		
	std::string line, key, x, y, z, uvX, uvY, uvZ, vnX, vnY, vnZ;
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
			face.vertices[0] = temp.x;
			face.normals[0] = temp.y;
			face.textureUV[0] = temp.z;
			temp = splitVertAndUv(y);
			face.vertices[1] = temp.x;
			face.normals[1] = temp.y;
			face.textureUV[1] = temp.z;
			temp = splitVertAndUv(z);
			face.vertices[2] = temp.x;
			face.normals[2] = temp.y;
			face.textureUV[2] = temp.z;

			model.faces.push_back(face);
		} else if (key == "vt") {
			Vec3f uv;

			iss >> uvX >> uvY;

			uv.x = std::stof(uvX);
			uv.y = std::stof(uvY);

			model.textureUV.push_back(uv);
		} else if (key == "vn") {
			Vec3f vn;

			iss >> vnX >> vnY >> vnZ;

			vn.x = std::stof(vnX);
			vn.y = std::stof(vnY);
			vn.z = std::stof(vnZ);

			model.normals.push_back(vn);
		}
	}
 
	file.close();

	render.models.push_back(&model);
	render.numberOfModels += 1;

	printf("[LOAD] Model %s took %f ms to load\n", pathname, timer.milliElapsed());
}

void loadTexture(Model &model, const char *pathname) {
	int width, height, nCh;

	Timer timer;
	unsigned char *data = stbi_load(pathname, &width, &height, &nCh, 0);

	if (!data) {
		printf("Couldn't find a texture %s\n", pathname);
		return;
	}

	Texture tex;

	tex.texture.width = width;
	tex.texture.height = height;
	tex.texture.bytepp = nCh;
	tex.texture.data = data;

	tex.texture.flip_vertically();

	model.texture = tex;
	printf("[LOAD] Texture %s took %f ms to load\n", pathname, timer.milliElapsed());
}