#pragma once

struct LightSource {
	Vec3f direction;
	Color color = Color(255, 255, 255);
	float intensity;
};

struct RendererState {
	Image imagebuffer;
	// TODO: Get rid of vector
	std::vector<Model *> models;
	int numberOfModels;
	float *zbuffer;
	LightSource light;
};

RendererState render;