#ifndef __STAR
#define __STAR

#include <libconfig.h++>
#include <Node.h>

class Star : public Node {
private:
	GLuint _sphereDisplayList;

	GLfloat _ambientLightColor[4];
	GLfloat _diffuseLightColor[4];
	GLfloat _specularLightColor[4];

public:
	double radius;

	Node *system;

	Star(const libconfig::Setting &star, Node *system);
	~Star();

	void draw();
};

#endif