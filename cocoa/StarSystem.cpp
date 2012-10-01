#include <iostream>

#include <StarSystem.h>

StarSystem::StarSystem() {
	std::cerr << "StarSystem initialized at position " << position << " relative to Universe center" << std::endl;

	this->star = new Star();
	Planet *planet = new Planet();

	this->star->position.z() = -149600000000.0;
	this->planets.push_front(planet);

	std::cerr << "Planet position is: " << planet->position << std::endl;
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
	// now draw the sun
	star->draw();

	// and the planets
	for (std::list<Planet *>::iterator i = planets.begin(); i != planets.end(); ++i) {
		Planet *p = *i;
		p->draw();
	}
}
