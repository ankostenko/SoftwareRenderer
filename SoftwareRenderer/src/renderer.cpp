void drawLine(Vec3f p1, Vec3f p2, float *zbuffer, Color &color) {
	viewport(p1, render.imagebuffer.width, render.imagebuffer.height);
	viewport(p2, render.imagebuffer.width, render.imagebuffer.height);

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

	int dx = x2 - x1;
	int dy = y2 - y1;
	int derror2 = std::abs(dy) * 2;
	int error2 = 0;
	int y = y1;

	for (int x = x1; x <= x2; x++) {
		float t = (x - x1) / (x2 - x1);
		float z = 1 / ((1 - t) * (p1.z) + t * (p2.z));
		if (steep) {
			if (x >= 0 && y >= 0 && y < render.imagebuffer.width && x < render.imagebuffer.height) {
				if (z < zbuffer[y + x * render.imagebuffer.width]) {
					zbuffer[y + x * render.imagebuffer.width] = z;
				}
			}
			render.imagebuffer.set(y, x, color);
		}
		else {
			if (x >= 0 && y >= 0 && x < render.imagebuffer.width && y < render.imagebuffer.height) {
				if (z < zbuffer[x + y * render.imagebuffer.width]) {
					zbuffer[x + y * render.imagebuffer.width] = z;
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

#define DEBUG_HAS_TEXTURE 1
void rasterize(Vec3f *triVert, Vec3f *globalVert, Texture texture, Vec3f *uv) {
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

	Vec3f lightDir = { -2.0f, -2.0f, -4.0f };
	Vec3f edgeNormal = norm(cross(globalVert[2] - globalVert[0], globalVert[1] - globalVert[0]));
	float intensity = norm(lightDir) * edgeNormal;
	if (intensity > 1.0f) {
		intensity = 1.0f;
	}
	if (intensity < 0) {
		intensity = -intensity * 0.3;
	}

	float area = edgeFunction(triVert[0], triVert[1], triVert[2]);
	if (area <= 0) {
		return;
	}

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
					// Texture mapping is working only after transformations done by MeshLab
					// It outputs completely different mapping and it does something but I'm not sure what
#if DEBUG_HAS_TEXTURE
					Vec3f uvC = uv[0] * w0 + uv[1] * w1 + uv[2] * w2;
					uvC = uvC * z;
					Color color = texture.texture.get(uvC.x, uvC.y);

					color.swapRBChannels();
					color.r *= intensity;
					color.g *= intensity;
					color.b *= intensity;
#else
					Color color(white.r * intensity, white.g * intensity, white.b * intensity);
#endif
					render.imagebuffer.set(x, y, color);
				}

			}
		}
	}
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
	//delete[] row;
}

void clearZBuffer(float farClippingPlane) {
	int size = render.imagebuffer.width * render.imagebuffer.height;
	for (int i = 0; i < size; i++) {
		render.zbuffer[i] = farClippingPlane;
	}
}

void initRenderer(int width, int height) {
	render.imagebuffer = Image(width, height);
	render.numberOfModels = 0;
	render.zbuffer = new float[width * height];
}