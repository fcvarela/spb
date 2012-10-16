#include <Common.h>
#include <Camera.h>

Camera::Camera() {
	std::cerr << "Camera alloc" << std::endl;
	this->max_angrate = 90.0;
	label = "CAMERA";
}

void Camera::step() {
	Vector3d direction = Vector3d(0.0, 0.0, 0.0);
	
	if (__keys__['W'] == 1)
		direction += Vector3d(0.0, 0.0, -1.0);

	if (__keys__['S'] == 1)
		direction += Vector3d(0.0, 0.0, 1.0);

	if (__keys__['A'] == 1)
		direction += Vector3d(-1.0, 0.0, 0.0);

	if (__keys__['D'] == 1)
		direction += Vector3d(1.0, 0.0, 0.0);

	if (__keys__['Q'] == 1)
		direction += Vector3d(0.0, 1.0, 0.0);

	if (__keys__['Z'] == 1)
		direction += Vector3d(0.0, -1.0, 0.0);

	target_velocity = direction * __camdelta__;
	acceleration = (target_velocity - velocity) * 20.0;
	velocity += acceleration * __dt__;
	Vector3d delta = velocity * __dt__;
	rotation.rotate(delta);
	position += delta;

	// process rotation of camera
	Vector3d angle = Vector3d(0.0, 0.0, 0.0);

	if (__keys__[GLFW_KEY_LEFT] == 1)
		angle += Vector3d(0.0, 1.0, 0.0);

	if (__keys__[GLFW_KEY_RIGHT] == 1)
		angle += Vector3d(0.0, -1.0, 0.0);

	if (__keys__[GLFW_KEY_UP] == 1)
		angle += Vector3d(-1.0, 0.0, 0.0);

	if (__keys__[GLFW_KEY_DOWN] == 1)
		angle += Vector3d(1.0, 0.0, 0.0);

	if (__keys__['C'] == 1)
		angle += Vector3d(0.0, 0.0, -1.0);

	if (__keys__['X'] == 1)
		angle += Vector3d(0.0, 0.0, 1.0);

	target_angrate = angle * 90.0;
	angaccel = (target_angrate - angrate) * 10.0;
	angrate += angaccel * __dt__;
	delta = angrate * __dt__;
	double deltalen = delta.length();
	if (deltalen > 1.0E-13) {
		delta.normalize();
		rotation = rotation * Quatd(delta, deltalen);
	}
}

void Camera::setPerspective() {
	GLdouble mat[16];

	this->rotation.glMatrix(mat);
	glMultMatrixd(mat);
	glTranslated(-position.x(), -position.y(), -position.z());
}
