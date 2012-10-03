#include <Common.h>
#include <Star.h>

Star::Star(const libconfig::Setting &star, Node *system) {
	this->system = system;
	star.lookupValue("name", this->label);
	star.lookupValue("radius", this->radius);
	this->position = system->position + Vector3d(star["position"][0], star["position"][1], star["position"][2]);

	GLfloat amb[4] = {.05, .05, .05, 1.0};
	GLfloat dif[4] = {.8, .8, .8, 1.0};
	GLfloat spe[4] = {.4, .4, .4, 1.0};

	memcpy(_ambientLightColor, amb, sizeof(GLfloat) * 4);
	memcpy(_diffuseLightColor, dif,  sizeof(GLfloat) * 4);
	memcpy(_specularLightColor, spe,  sizeof(GLfloat) * 4);

	// create the display list for this sphere
	GLUquadric *starq  = gluNewQuadric();
	gluQuadricNormals(starq, GL_SMOOTH);
	_sphereDisplayList = glGenLists(1);
	glNewList(_sphereDisplayList, GL_COMPILE);
	gluSphere(starq, this->radius, 50, 50);
	glEndList();
	gluDeleteQuadric(starq);
}

Star::~Star() {}

void Star::draw() {
	glPushMatrix();
	glTranslated(position.x(), position.y(), position.z());
	glColor4fv(_diffuseLightColor);
	glCallList(_sphereDisplayList);
	glPopMatrix();

	glDisable(GL_DEPTH_TEST);
	glColor3f(1.0, 0.0, 0.0);
	glRasterPos3f(position.x(), position.y(), position.z());
	__font__->Render(this->label.c_str());
	glEnable(GL_DEPTH_TEST);
}
