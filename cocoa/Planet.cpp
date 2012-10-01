#include <Planet.h>

Planet::Planet() {}
Planet::~Planet() {}

void Planet::draw() {
	glPushMatrix();
	glTranslated(position.x(), position.y(), position.z());
	glBegin(GL_POINTS);
	glVertex3d(0.0, 0.0, 0.0);
	glEnd();
	glPopMatrix();
}
