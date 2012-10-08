#ifndef __STAR
#define __STAR

#include <libconfig.h++>
#include <Node.h>

class StarSystem;

class Star : public Node {
private:
	GLuint _sphereDisplayList;

public:
	double radius;

	StarSystem *system;

	Star(const libconfig::Setting &star, StarSystem *system);
	~Star();

	void draw();
};

#endif