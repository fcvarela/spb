#include <Common.h>
#include <Camera.h>

Camera::Camera() {
	std::cerr << "Camera alloc" << std::endl;
}

void Camera::rotatex(double angle) {
	Quatd nrot(Vector3d(1.0, 0.0, 0.0), angle * __dt__);
	rotation = rotation * nrot;
}

void Camera::rotatey(double angle) {
	Quatd nrot(Vector3d(0.0, 1.0, 0.0), angle * __dt__);
	rotation = rotation * nrot;
}

void Camera::rotatez(double angle) {
	Quatd nrot(Vector3d(0.0, 0.0, 1.0), angle * __dt__);
	rotation = rotation * nrot;
}

void Camera::moveforward(double distance) {
	Vector3d delta = -Vector3d(0.0, 0.0, -1.0) * distance * __dt__;
	rotation.rotate(delta);
	position += delta;
}

void Camera::moveupward(double distance) {
	Vector3d delta = Vector3d(0.0, 1.0, 0.0) * distance * __dt__;
	rotation.rotate(delta);
	position += delta;
}

void Camera::straferight(double distance) {
	Vector3d delta = Vector3d(1.0, 0.0, 0.0) * distance * __dt__;
	rotation.rotate(delta);
	position += delta;
}

void Camera::step() {
	if (__keys__['W'] == 1)
		moveforward(__camdelta__);

	if (__keys__['S'] == 1)
		moveforward(-__camdelta__);

	if (__keys__['A'] == 1)
		straferight(-__camdelta__);

	if (__keys__['D'] == 1)
		straferight(__camdelta__);

	if (__keys__['Q'] == 1)
		moveupward(__camdelta__);

	if (__keys__['Z'] == 1)
		moveupward(-__camdelta__);

	if (__keys__[GLFW_KEY_LEFT] == 1)
		rotatey(25.0 * __dt__);

	if (__keys__[GLFW_KEY_RIGHT] == 1)
		rotatey(-25.0 * __dt__);

	if (__keys__[GLFW_KEY_UP] == 1)
		rotatex(25.0 * __dt__);

	if (__keys__[GLFW_KEY_DOWN] == 1)
		rotatex(-25.0 * __dt__);

	if (__keys__['C'] == 1)
		rotatez(25.0 * __dt__);

	if (__keys__['X'] == 1)
		rotatez(-25.0 * __dt__);

	//Node::step();
}

void Camera::setPerspective() {
	GLdouble mat[16];

	this->rotation.glMatrix(mat);
	glMultMatrixd(mat);
	glTranslated(-position.x(), -position.y(), -position.z());
}
