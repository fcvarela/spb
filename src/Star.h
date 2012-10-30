#ifndef __STAR
#define __STAR

#include <Node.h>

class StarSystem;

class Star : public Node {
public:
	double radius;
	StarSystem *system;
	GLuint _sphereDisplayList;
	Shader *shader;

	Star(StarSystem *system, double radius);
	~Star();

	void draw();
};

#endif
