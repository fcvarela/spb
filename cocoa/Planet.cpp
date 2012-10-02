#include <Planet.h>

Planet::Planet() {
	this->radius = 173814000.0;
	this->atmosphere_radius = this->radius * 1.025;

	GLUquadric *surface  = gluNewQuadric();
	gluQuadricNormals(surface, GL_SMOOTH);
	_sphereDisplayList = glGenLists(1);
	glNewList(_sphereDisplayList, GL_COMPILE);
	gluSphere(surface, this->radius, 50, 50);
	glEndList();
	gluDeleteQuadric(surface);
}

Planet::~Planet() {}

void Planet::draw() {
	glPushMatrix();
	glTranslated(position.x(), position.y(), position.z());
	glColor4f(1.0, 0.0, 0.0, 1.0);
	glCallList(_sphereDisplayList);
	glPopMatrix();
}
