#ifndef __NODE
#define __NODE

#include <Quaternion.h>
#include <Vector.h>

class Node {
public:
	Vector3d	position, velocity, acceleration;
	Quatd		rotation;

	Node();
	void step();
	void rotatex(double angle);
	void rotatey(double angle);
	void rotatez(double angle);
	void moveforward(double distance);
	void moveupward(double distance);
	void straferight(double distance);
	void stop();
};

#endif
