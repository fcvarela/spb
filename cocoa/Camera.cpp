#include <Camera.h>

Camera::Camera() {
	std::cerr << "Camera alloc" << std::endl;
	Vector3d initialPosition = Vector3d(0.0, 0.0, 1.0) * (12000000.0);

	this->position = initialPosition;
}

void Camera::setPerspective() {
	GLdouble mat[16];

	this->rotation.glMatrix(mat);
	glMultMatrixd(mat);
	glTranslated(-position.x(), -position.y(), -position.z());
}
