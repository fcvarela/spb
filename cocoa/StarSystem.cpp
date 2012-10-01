#include <iostream>

#include <StarSystem.h>

StarSystem::StarSystem() {
	std::cerr << "StarSystem initialized" << std::endl;

	star = new Star();


}

StarSystem::~StarSystem() {
	std::cerr << "StarSystem cleanup" << std::endl;

	delete star;
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

	// now draw the sun
	star->draw();

	// and the planets
	for (std::list<Planet *>::iterator i = planets.begin(); i != planets.end(); ++i) {
		Planet *p = *i;
		p->draw();
	}

	glPopMatrix();
}

