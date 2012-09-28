#include <Camera.h>

Camera::Camera() {
	std::cerr << "Camera alloc" << std::endl;
	Vector3d initialVelocity = Vector3d(0.0, 0.0, 0.0) * 7706.6; // avg ISS velocity in m/s
	Vector3d initialPosition = Vector3d(0.0, 0.0, 1.0) * (12000000.0); // 347 Km above surface

	this->position = initialPosition;
	this->velocity = initialVelocity;
}

void Camera::setPerspective() {
	GLdouble mat[16];

	this->rotation.glMatrix(mat);
	glMultMatrixd(mat);
	glTranslatef(-position.x(), -position.y(), -position.z());
}
