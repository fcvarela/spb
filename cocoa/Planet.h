#ifndef __PLANET
#define __PLANET

#include <Node.h>

class Planet : public Node {
public:
	double radius;
	double atmosphere_radius;

	GLuint _sphereDisplayList;

	Planet();
	~Planet();

	void draw();
};

#endif