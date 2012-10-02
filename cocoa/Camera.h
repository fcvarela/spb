#ifndef __CAMERA
#define __CAMERA

#include <Node.h>

class Camera : public Node {
public:
	double	max_angrate;
	Vector3d	target_velocity, target_acceleration;
	Vector3d	target_angrate, target_angaccel;

	Camera();
	~Camera();

	void step();
	void setPerspective();
};

#endif
