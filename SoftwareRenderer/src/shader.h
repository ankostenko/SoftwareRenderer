#pragma once

struct IShader {
	virtual Vec3f vertex(Vec3f vert, Vec3f normal, Vec3f light, int index) = 0;
	virtual Vec3f fragment(float w0, float w1, float w2, float z) = 0;
};

struct FlatShader : IShader {
	Mat4f uniform_MTI;
	Mat4f uniform_M;
	Mat4f uniform_VP;
	Vec3f uniform_LightPos;
	float varIntensity;
	Vec3f rgb = { 255.0f, 255.0f, 255.0f };

	virtual Vec3f vertex(Vec3f vert, Vec3f normal, Vec3f light, int index) override {
		Vec3f tranNormal = norm(normal * uniform_MTI);
		Vec3f lightDir = norm(uniform_LightPos - vert);
		varIntensity = clampMin(0.0f, tranNormal * lightDir);
		return vert * uniform_M * uniform_VP;
	}

	virtual Vec3f fragment(float w0, float w1, float w2, float z) override {
		return rgb * varIntensity;
	}
};

struct PhongShader : IShader {
	Mat4f uniform_M;
	Mat4f uniform_MTI;
	Mat4f uniform_VP;
	Vec3f uniform_ObjColor;
	Vec3f uniform_LightColor;
	Vec3f uniform_ViewPos;
	Vec3f uniform_LightPos;
	Vec3f Normal[3];
	Vec3f fragPos[3];

	virtual Vec3f vertex(Vec3f vert, Vec3f normal, Vec3f light, int index) override {
		Normal[index] = norm(normal * uniform_MTI);
		fragPos[index] = vert * uniform_M;

		return vert * uniform_M * uniform_VP;
	}

	virtual Vec3f fragment(float w0, float w1, float w2, float z) override {
		Vec3f interpNormal = norm(Normal[0] * w0 + Normal[1] * w1 + Normal[2] * w2);
		Vec3f interpFragPos = fragPos[0] * w0 + fragPos[1] * w1 + fragPos[2] * w2;
		
		Vec3f lightDir = norm(uniform_LightPos - interpFragPos);

		// Ambient
		float ambientStrength = 0.4f;
		Vec3f ambient = uniform_LightColor * ambientStrength;

		// Diffuse
		float diff = clampMinMax(-0.1f, 1.0f, dot(interpNormal, lightDir));
		Vec3f diffuse = uniform_LightColor * diff * 0.9f;

		// Specular
		float specularStrength = 0.3f;
		Vec3f reflectDir = reflect(lightDir, interpNormal);
		Vec3f viewDir = norm(uniform_ViewPos - interpFragPos);
		float spec = pow(clampMin(0.0f, dot(reflectDir, viewDir)), 16);
		Vec3f specular = uniform_LightColor * specularStrength * spec;

		Vec3f result = {  };
		result.x = (diffuse.x + ambient.x + specular.x) * uniform_ObjColor.x;
		result.y = (diffuse.y + ambient.y + specular.y) * uniform_ObjColor.y;
		result.z = (diffuse.z + ambient.z + specular.z) * uniform_ObjColor.z;
		
		return result;
	}
};

struct LightShader : IShader {
	Mat4f uniform_MVP;
	virtual Vec3f vertex(Vec3f vert, Vec3f normal, Vec3f light, int index) override {
		return vert * uniform_MVP;
	}

	virtual Vec3f fragment(float w0, float w1, float w2, float z) override {
		return { 255.0f, 255.0f, 255.0f };
	}
};

struct FullDiffuseShader : IShader {
	Mat4f uniform_MVP;

	virtual Vec3f vertex(Vec3f vert, Vec3f normal, Vec3f light, int index) override {
		return vert * uniform_MVP;
	}

	virtual Vec3f fragment(float w0, float w1, float w2, float z) override {
		return { 0.0, 255.0f, 0.0 };
	}
};