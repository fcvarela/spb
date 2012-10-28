#ifndef __PLANET
#define __PLANET

#include <list>
#include <Node.h>

class StarSystem;
class TerrainQuadtree;

class Planet : public Node {
public:
	double radius;
	double atmosphere_radius;
	double semimajor_axis;
	double eccentricity;
	double orbital_period;
	double orbital_inclination;

	double time_scale;

	// detail
	int maxlod;

	StarSystem *system;
	Node *parent;

	std::list<Planet *> moons;
	Shader *atmosphereShader;
	Shader *surfaceShader;

	TerrainQuadtree *quadtrees[6];

	GLuint _atmosphereDisplayList;
	GLuint _orbitDisplayList;

	Planet(StarSystem *system, Node *parent);
	~Planet();

	void draw();
	void drawDebug();
	void drawAtmosphere();
	void drawSurface();
	void step();
};

#endif
