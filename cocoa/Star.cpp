#include <Common.h>
#include <Star.h>

Star::Star() {
	GLfloat amb[4] = {.05, .05, .05, 1.0};
	GLfloat dif[4] = {.8, .8, .8, 1.0};
	GLfloat spe[4] = {.4, .4, .4, 1.0};

	memcpy(_ambientLightColor, amb, sizeof(GLfloat) * 4);
	memcpy(_diffuseLightColor, dif,  sizeof(GLfloat) * 4);
	memcpy(_specularLightColor, spe,  sizeof(GLfloat) * 4);

	// create the display list for this sphere
	this->radius = 6955E8;
	GLUquadric *star  = gluNewQuadric();
	gluQuadricNormals(star, GL_SMOOTH);
	_sphereDisplayList = glGenLists(1);
	glNewList(_sphereDisplayList, GL_COMPILE);
	gluSphere(star, this->radius, 50, 50);
	glEndList();
	gluDeleteQuadric(star);
}

Star::~Star() {}

void Star::draw() {
	glPushMatrix();
	glTranslated(position.x(), position.y(), position.z());
	glColor4fv(_diffuseLightColor);
	glCallList(_sphereDisplayList);
	glPopMatrix();
	
	/*
	glLightfv(GL_LIGHT0, GL_AMBIENT, _ambientLightColor);
	glLightfv(GL_LIGHT0, GL_AMBIENT, _diffuseLightColor);
	glLightfv(GL_LIGHT0, GL_AMBIENT, _specularLightColor);
	*/
}
