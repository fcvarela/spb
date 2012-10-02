#ifndef __NODE
#define __NODE

#include <Quaternion.h>
#include <Vector.h>

class Node {
public:
	Vector3d	position, velocity, acceleration;
	Quatd		rotation;
	Vector3d	angrate, angaccel;

	Node();
	virtual void step();
};

#endif
