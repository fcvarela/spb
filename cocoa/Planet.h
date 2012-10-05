#ifndef __PLANET
#define __PLANET

#include <libconfig.h++>
#include <list>
#include <Node.h>

class StarSystem;

class Planet : public Node {
public:
	double radius;
	double atmosphere_radius;
	double semimajor_axis;
	double eccentricity;
	double orbital_period;
	double orbital_inclination;

	double time_scale;

	StarSystem *system;
	Node *parent;

	std::list<Planet *> moons;
	Shader *atmosphereShader;
	Shader *surfaceShader;

	GLuint _surfaceDisplayList;
	GLuint _atmosphereDisplayList;
	GLuint _orbitDisplayList;

	Planet(const libconfig::Setting &planet, StarSystem *system, Node *parent);
	~Planet();

	void draw();
	void step();
};

#endif