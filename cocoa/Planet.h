#ifndef __PLANET
#define __PLANET

#include <libconfig.h++>
#include <Node.h>

class StarSystem;

class Planet : public Node {
public:
	double radius;
	double atmosphere_radius;
	double semimajor_axis;
	double eccentricity;
	double orbital_period;

	double time_scale;

	StarSystem *system;
	Shader *atmosphereShader;
	Shader *surfaceShader;

	GLuint _surfaceDisplayList;
	GLuint _atmosphereDisplayList;

	Planet(const libconfig::Setting &planet, StarSystem *system);
	~Planet();

	void draw();
	void step();
};

#endif