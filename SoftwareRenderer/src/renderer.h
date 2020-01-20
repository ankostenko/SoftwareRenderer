#pragma once

struct LightSource {
	Vec3f position;
	Color color = Color(255, 255, 255);
	float intensity;
};

struct AntiAliasingBuffers {
	Image imagebuffer;
	float* zbuffer;
	int aaCoeff;
};

struct RendererState {
	Image imagebuffer;
	// TODO: Get rid of vector
	std::vector<Model *> models;
	int numberOfModels;
	float *zbuffer;
	LightSource light;
	AntiAliasingBuffers aaBuffers;
};

RendererState render;