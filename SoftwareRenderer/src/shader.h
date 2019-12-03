#pragma once

struct IShader {
	virtual Vec3f vertex(Vec3f vert, Vec3f normal, Vec3f light) = 0;
	virtual Vec3f fragment() = 0;
};

struct FlatShader : IShader {
	Mat4f uniform_M;
	Mat4f uniform_VP;
	float varIntensity;
	Vec3f rgb = { 255.0f, 255.0f, 255.0f };

	virtual Vec3f vertex(Vec3f vert, Vec3f normal, Vec3f light) override {
		normal = norm(normal * inverse(uniform_M));
		varIntensity = clampMin(0.0f, normal * light);
		return vert * uniform_M * uniform_VP;
	}

	virtual Vec3f fragment() override {
		return rgb * varIntensity;
	}
};

struct LightShader : IShader {
	Mat4f uniform_MVP;
	virtual Vec3f vertex(Vec3f vert, Vec3f normal, Vec3f light) override {
		return vert * uniform_MVP;
	}

	virtual Vec3f fragment() override {
		return { 255.0f, 255.0f, 255.0f };
	}
};