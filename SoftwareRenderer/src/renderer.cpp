void drawLine(Vec3f p1, Vec3f p2, Color &color) {
	viewport(p1, render.imagebuffer.width, render.imagebuffer.height);
	viewport(p2, render.imagebuffer.width, render.imagebuffer.height);

	if (p1.z < -1.0f || p1.z > 1.0f) { return; }
	if (p2.z < -1.0f || p2.z > 1.0f) { return; }

	float x1 = p1.x;
	float y1 = p1.y;
	float x2 = p2.x;
	float y2 = p2.y;
	p1.z = 1 / p1.z;
	p2.z = 1 / p2.z;

	bool steep = false;
	if (std::abs(x1 - x2) < std::abs(y1 - y2)) {
		std::swap(x1, y1);
		std::swap(x2, y2);
		steep = true;
	}

	if (x1 > x2) {
		std::swap(x1, x2);
		std::swap(y1, y2);
		std::swap(p1.z, p2.z);
	}

	int dx = int(x2 - x1);
	int dy = int(y2 - y1);
	int derror2 = std::abs(dy) * 2;
	int error2 = 0;
	int y = int(y1);

	if (x1 < 0) { x1 = 0.0f; }
	if (x1 > render.imagebuffer.width) { x1 = float(render.imagebuffer.width); }
	if (x2 < 0) { x2 = 0.0f; }
	if (x2 > render.imagebuffer.width) { x2 = float(render.imagebuffer.width); }

	for (int x = x1; x <= x2; x++) {
		float t = (x - x1) / (x2 - x1);
		float z = 1 / ((1 - t) * (p1.z) + t * (p2.z));
		if (steep) {
			if (x >= 0 && y >= 0 && y < render.imagebuffer.width && x < render.imagebuffer.height) {
				if (z < render.zbuffer[y + x * render.imagebuffer.width]) {
					render.zbuffer[y + x * render.imagebuffer.width] = z;
				}
			}
			render.imagebuffer.set(y, x, color);
		}
		else {
			if (x >= 0 && y >= 0 && x < render.imagebuffer.width && y < render.imagebuffer.height) {
				if (z < render.zbuffer[x + y * render.imagebuffer.width]) {
					render.zbuffer[x + y * render.imagebuffer.width] = z;
				}
			}
			render.imagebuffer.set(x, y, color);
		}
		error2 += derror2;
		if (error2 > dx) {
			y += (y2 > y1 ? 1 : -1);
			error2 -= dx * 2;
		}
	}
}

float edgeFunction(const Vec3f &a, const Vec3f &b, const Vec3f &c) {
	return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
}

#define DEBUG_HAS_TEXTURE 0
#define AA 1
void rasterize(Vec3f* triVert, IShader& shader) {
	// z-cullling
	if (triVert[0].z < -1.0f || triVert[0].z > 1.0f) { return; }
	if (triVert[1].z < -1.0f || triVert[1].z > 1.0f) { return; }
	if (triVert[2].z < -1.0f || triVert[2].z > 1.0f) { return; }

	triVert[0].z = 1 / triVert[0].z;
	triVert[1].z = 1 / triVert[1].z;
	triVert[2].z = 1 / triVert[2].z;

#if DEBUG_HAS_TEXTURE
	uv[0] = uv[0] * triVert[0].z;
	uv[1] = uv[1] * triVert[1].z;
	uv[2] = uv[2] * triVert[2].z;
#endif

	// Find "border" box
	int minX = triVert[0].x;
	int minY = triVert[0].y;
	int maxX = triVert[0].x;
	int maxY = triVert[0].y;

	for (int i = 1; i < 3; i++) {
		if (triVert[i].x < minX) {
			minX = triVert[i].x;
		}
		if (triVert[i].x > maxX) {
			maxX = triVert[i].x;
		}
		if (triVert[i].y < minY) {
			minY = triVert[i].y;
		}
		if (triVert[i].y > maxY) {
			maxY = triVert[i].y;
		}
	}

	int width = render.imagebuffer.width;
	int height = render.imagebuffer.height;

	if ((triVert[0].x > width || triVert[0].x < 0 || triVert[0].y > height || triVert[0].y < 0) &&
		(triVert[1].x > width || triVert[1].x < 0 || triVert[1].y > height || triVert[1].y < 0) &&
		(triVert[2].x > width || triVert[2].x < 0 || triVert[2].y > height || triVert[2].y < 0)) {
		return;
	}

	float area = edgeFunction(triVert[0], triVert[1], triVert[2]);
	if (area <= 0) {
		return;
	}
#if AA
	static const Vec3f offsets[] = {
		Vec3f({-0.125f, -0.375f, 0.0f}),
		Vec3f({ 0.375f, -0.125f, 0.0f}),
		Vec3f({-0.375f,  0.125f, 0.0f}),
		Vec3f({ 0.125f,  0.375f, 0.0f}),
	};

	int pxIndex = 0;
	for (int y = minY; y <= maxY; y++) {
		for (int x = minX; x <= maxX; x++) {
			pxIndex = x + y * render.imagebuffer.width;
			pxIndex *= 4;
			for (int aaIndex = 0; aaIndex < render.aaBuffers.aaCoeff; aaIndex++, pxIndex++) {
				Vec3f p = { x + offsets[aaIndex].x, y + offsets[aaIndex].y, 0 };

				float w0 = edgeFunction(triVert[1], triVert[2], p);
				float w1 = edgeFunction(triVert[2], triVert[0], p);
				float w2 = edgeFunction(triVert[0], triVert[1], p);

				if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
					// Barycentric coordinates
					w0 /= area;
					w1 /= area;
					w2 /= area;

					float z = 1 / (triVert[0].z * w0 + triVert[1].z * w1 + triVert[2].z * w2);

					// Clipping
					if (p.x >= render.imagebuffer.width || p.x < 0 || p.y >= render.imagebuffer.height || p.y < 0) {
						continue;
					}

					int sizeMultiplier = (render.aaBuffers.aaCoeff / 2);
					//if (z < render.aaBuffers.zbuffer[int(p.x) + int(p.y) * render.aaBuffers.imagebuffer.width]) {
						render.aaBuffers.zbuffer[int(p.x) + int(p.y) * render.aaBuffers.imagebuffer.width] = z;
						Vec3f resColor = shader.fragment(w0, w1, w2, z);
						resColor.x = clampMinMax(0, 255, resColor.x);
						resColor.y = clampMinMax(0, 255, resColor.y);
						resColor.z = clampMinMax(0, 255, resColor.z);
						Color color(resColor.x, resColor.y, resColor.z);
						render.aaBuffers.imagebuffer.set(pxIndex, color);
					//}
				}
			}
		}
	}
#else
	for (int y = minY; y <= maxY; y++) {
		for (int x = minX; x <= maxX; x++) {
			Vec3f p = { x, y, 0 };
			// Determine whether point inside the triangle or not
			float w0 = edgeFunction(triVert[1], triVert[2], p);
			float w1 = edgeFunction(triVert[2], triVert[0], p);
			float w2 = edgeFunction(triVert[0], triVert[1], p);

			if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
				// Barycentric coordinates
				w0 /= area;
				w1 /= area;
				w2 /= area;

				float z = 1 / (triVert[0].z * w0 + triVert[1].z * w1 + triVert[2].z * w2);

				// Clipping
				if (x >= render.imagebuffer.width || x < 0 || y >= render.imagebuffer.height || y < 0) {
					continue;
				}

				if (z < render.zbuffer[x + y * render.imagebuffer.width]) {
					render.zbuffer[x + y * render.imagebuffer.width] = z;
					// TODO: Investigate what the hell is going on with texture mapping
					// Texture mapping works only after transformations done by MeshLab
					// It outputs completely different mapping and it does something but I'm not sure what
					//Vec3f normal = normals[0] * w0 + normals[1] * w1 + normals[2] * w2;
					//normal = normal * z;
					//normal.normalize();

					// Program counts properties of a material and light to render a shaded point
					//char r = render.light.color.r * (clampMinMax(0.25f, 1.0f, render.light.intensity * maxf(0.0f, (render.light.direction * normal))));
					//char g = render.light.color.g * (clampMinMax(0.25f, 1.0f, render.light.intensity * maxf(0.0f, (render.light.direction * normal))));
					//char b = render.light.color.b * (clampMinMax(0.25f, 1.0f, render.light.intensity * maxf(0.0f, (render.light.direction * normal))));
#if DEBUG_HAS_TEXTURE
					Vec3f uvC = uv[0] * w0 + uv[1] * w1 + uv[2] * w2;
					uvC = uvC * z;
					Color color = texture.texture.get(uvC.x, uvC.y);

					//color.swapRBChannfels();
					//color.r *= intensity;
					//color.g *= intensity;
					//color.b *= intensity;
#else
					Vec3f resColor = shader.fragment(w0, w1, w2, z);
					resColor.x = clampMinMax(0, 255, resColor.x);
					resColor.y = clampMinMax(0, 255, resColor.y);
					resColor.z = clampMinMax(0, 255, resColor.z);
					Color color(resColor.x, resColor.y, resColor.z);
#endif
					render.imagebuffer.set(x, y, color);
				}
			}
		}
	}
#endif
}

void clearImBuffer(Color color) {
	int size = render.imagebuffer.width * render.imagebuffer.bytepp;
	static unsigned char *row = new unsigned char[size];
	for (int i = 0; i < render.imagebuffer.width; i++) {
		memmove(&row[i * render.imagebuffer.bytepp], color.rgba, render.imagebuffer.bytepp);
	}

	for (int y = 0; y < render.imagebuffer.height; y++) {
		memmove(&render.imagebuffer.data[y * size], row, size);
	}

	size = size * render.aaBuffers.aaCoeff / 2;
	static unsigned char* aaRow = new unsigned char[size];
	int sizeMultiplier = render.aaBuffers.aaCoeff / 2;
	for (int i = 0; i < render.aaBuffers.imagebuffer.width * sizeMultiplier; i++) {
		memmove(&aaRow[i * render.aaBuffers.imagebuffer.bytepp], color.rgba, render.aaBuffers.imagebuffer.bytepp);
	}

	for (int y = 0; y < render.imagebuffer.height * sizeMultiplier; y++) {
		memmove(&render.aaBuffers.imagebuffer.data[y * size], aaRow, size);
	}
}

void clearZBuffer(float farClippingPlane) {
	int size = render.imagebuffer.width * render.imagebuffer.height;
	for (int i = 0; i < size; i++) {
		render.zbuffer[i] = farClippingPlane;
	}

	size *= render.aaBuffers.aaCoeff;
	
	for (int i = 0; i < size; i++) {
		render.aaBuffers.zbuffer[i] = farClippingPlane;
	}

}

void initRenderer(int width, int height, Vec3f lightDir) {
	render.imagebuffer = Image(width, height, 1);
	render.numberOfModels = 0;
	render.zbuffer = new float[width * height];

	render.aaBuffers.aaCoeff = 4;
	render.aaBuffers.zbuffer = new float[width * height * render.aaBuffers.aaCoeff];
	render.aaBuffers.imagebuffer = Image(width, height, render.aaBuffers.aaCoeff);
	memset(render.aaBuffers.imagebuffer.data, 0, render.aaBuffers.imagebuffer.size() * render.aaBuffers.aaCoeff);

	render.light.position = norm(lightDir);
	render.light.intensity = 3.5f;
	render.light.color = white;
}

void drawModel(Model& model, IShader& shader) {
	for (int i = 0; i < model.facesNumber(); i++) {
		Vec3f triVert[3];
		Vec3f textureUV[3];
		Vec3f normals[3];

		for (int j = 0; j < 3; j++) {
			triVert[j] = model.triVert(i, j);
			normals[j] = model.triNorm(i, j);
			triVert[j] = shader.vertex(triVert[j], normals[j], render.light.position, j);
			triVert[j].perspectiveDivide();
			viewport(triVert[j], render.imagebuffer.width, render.imagebuffer.height);
		}
		rasterize(triVert, shader);
	}
	
#if AA
	int size = render.aaBuffers.imagebuffer.width * render.aaBuffers.imagebuffer.height;
	for (int y = 0, iy = 0; y < size * render.aaBuffers.aaCoeff; y += 4, iy += 1) {
		Color t0 = render.aaBuffers.imagebuffer.get(y + 0);
		Color t1 = render.aaBuffers.imagebuffer.get(y + 1);
		Color t2 = render.aaBuffers.imagebuffer.get(y + 2);
		Color t3 = render.aaBuffers.imagebuffer.get(y + 3);
	
		Color avg((t0.r + t1.r + t2.r + t3.r) / 4, (t0.g + t1.g + t2.g + t3.g) / 4, (t0.b + t1.b + t2.b + t3.b) / 4);
		render.imagebuffer.set(iy, avg);
	}
#endif
}