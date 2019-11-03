#pragma once

struct RendererState {
	Image imagebuffer;
	// TODO: Get rid of vector
	std::vector<Model *> models;
	int numberOfModels;
	float *zbuffer;
};

static RendererState render;