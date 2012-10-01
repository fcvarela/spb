#ifndef __NODE
#define __NODE

#include <Quaternion.h>
#include <Vector.h>

class Node {
public:
	Vector3d	position, velocity, acceleration, angrate, angaccel;
	Quatd		rotation;

	Node();
	virtual void step();
	void stop(bool full);
};

#endif
