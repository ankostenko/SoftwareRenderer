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
		view = inverse(::lookAt(position, target));
	}

	void invView() {
		view = inverse(view);
	}
	
	Mat4f project() {
		proj = projection(fov, render.imagebuffer.width / (float)render.imagebuffer.height, nearClippingPlane, farClippingPlane);
		return proj;
	}
};

struct FreeCamera {
	Vec3f position;
	Vec3f front;
	Vec3f up;
	Vec3f right;
	Vec3f worldUp;
	float nearClippingPlane;
	float farClippingPlane;
	float fov;
	Mat4f view;
	Mat4f proj;
	float yaw = -90.0f;
	float pitch = 0.0f;

	int lastMouseY;
	int lastMouseX;
	float mouseSensitivity = 0.1f;
	float movementSpeed = 1.0f;

	FreeCamera(float nearPlane, float farPlane, float fov, Vec3f pos = Vec3f({ 3.0f, 0.0f, 3.0f })) : position(pos), fov(fov), nearClippingPlane(nearPlane),
		farClippingPlane(farPlane), worldUp(Vec3f({ 0.0f, 1.0f, 0.0f })) {
		mouse.wheelFOV = fov;
		updateVectors();
	}

	void lookAt() {
		view = inverse(::lookAt(position, position + front, up));
	}

	Mat4f project() {
		proj = projection(radians(fov), render.imagebuffer.width / (float)render.imagebuffer.height, nearClippingPlane, farClippingPlane);
		return proj;
	}
	
	void processMouseInput(int newMouseX, int newMouseY, float deltaTime) {
		int xOffset = -(lastMouseX - newMouseX);
		int yOffset = lastMouseY - newMouseY;

		yaw += (float)xOffset * mouseSensitivity * deltaTime;
		pitch += (float)yOffset * mouseSensitivity * deltaTime;

		lastMouseX = newMouseX;
		lastMouseY = newMouseY;
	}

	void processMouseScrolling(Mouse mouse) {
		fov = mouse.wheelFOV;
	}

	void forwardMovement(int direction, float deltaTime) {
		position = position + front * movementSpeed * deltaTime * direction;
	}

	void rightMovement(int direction, float deltaTime) {
		position = position + right * movementSpeed * deltaTime * direction;
	}

	void updateVectors() {
		front.x = cos(radians(yaw)) * cos(radians(pitch));
		front.y = sin(radians(pitch));
		front.z = cos(radians(pitch)) * sin(radians(yaw));
		front = norm(front);

		right = norm(cross(front, worldUp));
		up = norm(cross(right, front));
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