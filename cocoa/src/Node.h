#ifndef __NODE
#define __NODE

#include <Quaternion.h>
#include <Vector.h>
#include <Shader.h>

class Node {
public:
	Vector3d	position, velocity, acceleration;
	Quatd		rotation;
	Vector3d	angrate, angaccel;
	std::string	label;

	Shader		*shader;

	Node();
	virtual void step();
	virtual void draw();
};

#endif
