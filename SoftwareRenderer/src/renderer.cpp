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

	int dx = x2 - x1;
	int dy = y2 - y1;
	int derror2 = std::abs(dy) * 2;
	int error2 = 0;
	int y = y1;

	if (x1 < 0) { x1 = 0; }
	if (x1 > render.imagebuffer.width) { x1 = render.imagebuffer.width; }
	if (x2 < 0) { x2 = 0; }
	if (x2 > render.imagebuffer.width) { x2 = render.imagebuffer.width; }

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
void rasterize(Vec3f *triVert, IShader &shader) {
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
}

void clearZBuffer(float farClippingPlane) {
	int size = render.imagebuffer.width * render.imagebuffer.height;
	for (int i = 0; i < size; i++) {
		render.zbuffer[i] = farClippingPlane;
	}
}

void initRenderer(int width, int height, Vec3f lightPos) {
	render.imagebuffer = Image(width, height);
	render.numberOfModels = 0;
	render.zbuffer = new float[width * height];

	render.light.position = lightPos;
	render.light.intensity = 3.5f;
	render.light.color = white;
}

void drawModel(Model &model, IShader &shader) {
	for (int i = 0; i < model.facesNumber(); i++) {
		Vec3f triVert[3];
		Vec3f textureUV[3];
		Vec3f normals[3];

		//for (int j = 0; j < 3; j++) {
		//	triVert[j] = model.triVert(i, j);
		//	normals[j] = model.triNorm(i, j);
		//	triVert[j] = shader.vertex(triVert[j], normals[j], j);
		//	viewport(triVert[j], render.imagebuffer.width, render.imagebuffer.height);
		//}
		triVert[0] = model.triVert(i, 0);
		triVert[1] = model.triVert(i, 1);
		triVert[2] = model.triVert(i, 2);
		
		normals[0] = model.triNorm(i, 0);
		normals[1] = model.triNorm(i, 1);
		normals[2] = model.triNorm(i, 2);

		triVert[0] = shader.vertex(triVert[0], normals[0], 0);
		triVert[1] = shader.vertex(triVert[1], normals[1], 1);
		triVert[2] = shader.vertex(triVert[2], normals[2], 2);
		
		viewport(triVert[0], render.imagebuffer.width, render.imagebuffer.height);
		viewport(triVert[1], render.imagebuffer.width, render.imagebuffer.height);
		viewport(triVert[2], render.imagebuffer.width, render.imagebuffer.height);
		
		rasterize(triVert, shader);
	}
}

// Basic ui system
void DrawRectangle(int x, int y, int width, int height, Color color) {
	for (int _x = x; _x < x + width; _x++) {
		for (int _y = y; _y < y + height; _y++) {
			render.imagebuffer.set(_x, _y, color);
		}
	}
}

const int M = 3;
void DrawZero(int x, int y, int width, int height, Color color) {
	DrawRectangle(x, y, width, M, color);
	DrawRectangle(x + width, y, M, height, color);
	DrawRectangle(x, y + height - M, width, M, color);
	DrawRectangle(x, y, M, height, color);
}

void DrawOne(int x, int y, int width, int height, Color color) {
	DrawRectangle(x + width - M, y, M, height, color);
}

void DrawTwo(int x, int y, int width, int height, Color color) {
	DrawRectangle(x, y, width, M, color);
	DrawRectangle(x + width - M, y, M, height / 2, color);
	DrawRectangle(x, y + height / 2 - M, width, M, color);
	DrawRectangle(x, y + height / 2 - M, M, height / 2, color);
	DrawRectangle(x, y + height - M, width, M, color);
}

void DrawThree(int x, int y, int width, int height, Color color) {
	DrawRectangle(x, y, width, M, color);
	DrawRectangle(x + width - M, y, M, height / 2, color);
	DrawRectangle(x, y + height / 2 - M, width, M, color);
	DrawRectangle(x + width - M, y + height / 2 - M, M, height / 2, color);
	DrawRectangle(x, y + height - M, width, M, color);
}

void DrawFive(int x, int y, int width, int height, Color color) {
	DrawRectangle(x, y, width, M, color);
	DrawRectangle(x, y, M, height / 2, color);
	DrawRectangle(x, y + height / 2 - M, width, M, color);
	DrawRectangle(x + width - M, y + height / 2 - M, M, height / 2, color);
	DrawRectangle(x, y + height - M, width, M, color);
}

void DrawSix(int x, int y, int width, int height, Color color) {
	DrawRectangle(x, y, width, M, color);
	DrawRectangle(x, y, M, height / 2, color);
	DrawRectangle(x, y + height / 2 - M, width, M, color);
	DrawRectangle(x, y + height / 2 - M, M, height / 2, color);
	DrawRectangle(x + width - M, y + height / 2 - M, M, height / 2, color);
	DrawRectangle(x, y + height - M, width, M, color);
}

void DrawNine(int x, int y, int width, int height, Color color) {
	DrawRectangle(x, y, width, M, color);
	DrawRectangle(x + width - M, y, M, height / 2, color);
	DrawRectangle(x, y, M, height / 2, color);
	DrawRectangle(x, y + height / 2 - M, width, M, color);
	DrawRectangle(x + width - M, y + height / 2 - M, M, height / 2, color);
	DrawRectangle(x, y + height - M, width, M, color);
}

void DrawEight(int x, int y, int width, int height, Color color) {
	DrawRectangle(x, y, width, M, color);
	DrawRectangle(x + width - M, y, M, height / 2, color);
	DrawRectangle(x, y, M, height / 2, color);
	DrawRectangle(x, y + height / 2 - M, width, M, color);
	DrawRectangle(x, y + height / 2 - M, M, height / 2, color);
	DrawRectangle(x + width - M, y + height / 2 - M, M, height / 2, color);
	DrawRectangle(x, y + height - M, width, M, color);
}

void DrawSeven(int x, int y, int width, int height, Color color) {
	DrawRectangle(x + width - M, y, M, height, color);
	DrawRectangle(x, y, width, M, color);
}

void DrawFour(int x, int y, int width, int height, Color color) {
	DrawRectangle(x, y, M, height / 2, color);
	DrawRectangle(x, y + height / 2, width, M, color);
	DrawRectangle(x + width - M, y, M, height, color);
}

const int MARGIN = 10;
const int WIDTH = 30;
const int HEIGHT = 40;
void DrawScore(int x, int y, int score) {
	int temp = score;
	int NumberOfDigits = 0;
	while (temp) {
		temp = temp / 10;
		NumberOfDigits++;
	}

	int scorecopy = score;
	int totalWidth = NumberOfDigits * (WIDTH + 2 * MARGIN);
	
	for (int i = 0; i < NumberOfDigits; i++) {
		int r = scorecopy % 10;
		scorecopy /= 10;
		switch (r) {
			case 0: { DrawZero( x + totalWidth / 4 - i * (MARGIN * 2 + WIDTH), y, WIDTH, HEIGHT, white); } break; 
			case 1: { DrawOne(  x + totalWidth / 4 - i * (MARGIN * 2 + WIDTH), y, WIDTH, HEIGHT, white); } break; 
			case 2: { DrawTwo(  x + totalWidth / 4 - i * (MARGIN * 2 + WIDTH), y, WIDTH, HEIGHT, white); } break; 
			case 3: { DrawThree(x + totalWidth / 4 - i * (MARGIN * 2 + WIDTH), y, WIDTH, HEIGHT, white); } break; 
			case 4: { DrawFour( x + totalWidth / 4 - i * (MARGIN * 2 + WIDTH), y, WIDTH, HEIGHT, white); } break; 
			case 5: { DrawFive( x + totalWidth / 4 - i * (MARGIN * 2 + WIDTH), y, WIDTH, HEIGHT, white); } break; 
			case 6: { DrawSix(  x + totalWidth / 4 - i * (MARGIN * 2 + WIDTH), y, WIDTH, HEIGHT, white); } break; 
			case 7: { DrawSeven(x + totalWidth / 4 - i * (MARGIN * 2 + WIDTH), y, WIDTH, HEIGHT, white); } break; 
			case 8: { DrawEight(x + totalWidth / 4 - i * (MARGIN * 2 + WIDTH), y, WIDTH, HEIGHT, white); } break; 
			case 9: { DrawNine( x + totalWidth / 4 - i * (MARGIN * 2 + WIDTH), y, WIDTH, HEIGHT, white); } break;
		}
	}
}
