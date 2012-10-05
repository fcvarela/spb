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
	if (target_velocity != velocity)
		acceleration = (target_velocity - velocity) * 30.0;
	else
		acceleration = Vector3d(0.0, 0.0, 0.0);

	Vector3d posdelta = velocity * __dt__;
	rotation.rotate(posdelta);

	velocity += acceleration * __dt__;
	position += posdelta;

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

	target_angrate = angle * 40.0;
	if (target_angrate != angrate)
		angaccel = (target_angrate - angrate) * 30.0;
	else
		angaccel = Vector3d(0.0, 0.0, 0.0);

	Vector3d angdelta = angrate * __dt__;
	angrate += angaccel * __dt__;

	if (angdelta != Vector3d(0.0, 0.0, 0.0)) {
		double angdeltalen = angdelta.length();
		angdelta.normalize();
		Quatd nrot(angdelta, angdeltalen);
		rotation = rotation * nrot;
	}

	// make sure we lock when sufficiently close to zero
	if (velocity.length() < 10E-12)
		velocity = Vector3d(0.0, 0.0, 0.0);
	if (acceleration.length() < 10E-12)
		velocity = Vector3d(0.0, 0.0, 0.0);
	if (angrate.length() < 10E-12)
		angrate = Vector3d(0.0, 0.0, 0.0);
	if (angaccel.length() < 10E-12)
		angaccel = Vector3d(0.0, 0.0, 0.0);
}

void Camera::setPerspective() {
	GLdouble mat[16];

	this->rotation.glMatrix(mat);
	glMultMatrixd(mat);
	glTranslated(-position.x(), -position.y(), -position.z());
}
