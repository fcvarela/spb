#ifndef __STAR
#define __STAR

#include <Node.h>

class StarSystem;

class Star : public Node {
private:
	GLuint _sphereDisplayList;

public:
	double radius;
	StarSystem *system;

	Star(StarSystem *system, double radius);
	~Star();

	void draw();
};

#endif
