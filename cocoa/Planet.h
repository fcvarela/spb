#ifndef __PLANET
#define __PLANET

#include <libconfig.h++>
#include <Node.h>

class Planet : public Node {
public:
	double radius;
	double atmosphere_radius;
	double semimajor_axis;
	double eccentricity;
	double orbital_period;

	double time_scale;

	Node *system;

	GLuint _sphereDisplayList;

	Planet(const libconfig::Setting &planet, Node *system);
	~Planet();

	void draw();
	void step();
};

#endif