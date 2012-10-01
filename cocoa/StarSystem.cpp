#include <iostream>

#include <StarSystem.h>

StarSystem::StarSystem() {
	std::cerr << "StarSystem initialized at position " << position << " relative to Universe center" << std::endl;

	this->star = new Star();
	Planet *planet = new Planet();

	planet->position.z() = 149600000000.0;
	this->planets.push_front(planet);

	std::cerr << "Planet position is: " << planet->position << std::endl;
}

StarSystem::~StarSystem() {
	std::cerr << "StarSystem cleanup" << std::endl;

	//delete star;
	//delete planets;
}

void StarSystem::step() {
	// reposition sun
	star->step();

	// reposition planets
	for (std::list<Planet *>::iterator i = planets.begin(); i != planets.end(); ++i) {
		Planet *p = *i;
		p->step();
	}
}

void StarSystem::draw() {
	// draw star, planets
	glPushMatrix();
	glTranslated(position.x(), position.y(), position.z());

	// debug
	std::list<Planet *>::iterator p = planets.begin();
	glColor4f(1.0, 0.0, 0.0, 1.0);
	glBegin(GL_LINES);
	glVertex3d(star->position.x(), star->position.y(), star->position.z());
	glVertex3d((*p)->position.x(), (*p)->position.y(), (*p)->position.z());
	glEnd();

	// now draw the sun
	star->draw();

	// and the planets
	for (std::list<Planet *>::iterator i = planets.begin(); i != planets.end(); ++i) {
		Planet *p = *i;
		p->draw();
	}

	glPopMatrix();
}
