struct PerspectiveCamera {
	Vec3f staticPosition;
	Vec3f position;
	float nearClippingPlane;
	float farClippingPlane;
	float fov;
	Mat4f view;
	Mat4f proj;

	PerspectiveCamera(float nearPlane, float farPlane, float fov, Vec3f pos = Vec3f({ 0.0f, 0.0f, 0.0f })) : position(pos), staticPosition(pos),
		nearClippingPlane(nearPlane), fov(fov), farClippingPlane(farPlane) {}

	void lookAt(Vec3f target) {
		view = ::lookAt(position, target);
	}

	void invView() {
		view = inverse(view);
	}
	
	Mat4f project() {
		proj = projection(fov, render.imagebuffer.width / (float)render.imagebuffer.height, nearClippingPlane, farClippingPlane);
		return proj;
	}
};

struct OrthographicCamera {
	Vec3f staticPosition;
	Vec3f position;
	float nearClippingPlane;
	float farClippingPlane;
	float fov;
	Mat4f view;
	Mat4f proj;

	OrthographicCamera(float nearPlane, float farPlane, float fov, Vec3f pos = Vec3f({ 0.0f, 0.0f, 0.0f })) : position(pos), staticPosition(pos),
		nearClippingPlane(nearPlane), farClippingPlane(farPlane), fov(fov) { }

	void lookAt(Vec3f target) {
		view = ::lookAt(position, target);
	}

	void invView() {
		view = inverse(view);
	}

	Mat4f project() {
		proj = orthoProjection(fov, render.imagebuffer.width / (float)render.imagebuffer.height, nearClippingPlane, farClippingPlane);
		return proj;
	}
};