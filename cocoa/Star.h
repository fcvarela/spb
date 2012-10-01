#ifndef __STAR
#define __STAR

#include <Node.h>

class Star : public Node {
private:
	GLuint _sphereDisplayList;

	GLfloat _ambientLightColor[4];
	GLfloat _diffuseLightColor[4];
	GLfloat _specularLightColor[4];

public:
	double radius;

	Star();
	~Star();

	void draw();
};

#endif