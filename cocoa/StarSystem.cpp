#include <iostream>

#include <StarSystem.h>

StarSystem::StarSystem() {
	this->star = new Star();
	Planet *planet = new Planet();

	star->position = this->position;
	planet->position.x() = this->position.x()-149059708700700.0;

	planet->label = std::string("EARTH");
	this->planets.push_front(planet);
}

StarSystem::~StarSystem() {
	std::cerr << "StarSystem cleanup" << std::endl;

	delete star;
	//delete planets;
}

// returns nearest node from node given
// used to recalc every position for OpenGL coordinate size
Node *StarSystem::nearestNode(Node *node) {
	double dist;
	Node *nearest = star;
	double shortest_distance = (star->position - node->position).length();

	// iterate planets
	for (std::list<Planet *>::iterator i = planets.begin(); i != planets.end(); ++i) {
		Planet *p = *i;
		dist = (p->position - node->position).length();
		if (dist < shortest_distance) {
			shortest_distance = dist;
			nearest = p;
		}
	}

	return nearest;
}

// effectively moves the center of origin to the closest node
void StarSystem::recalculatePositions(Vector3d &subtract) {
	this->position = this->position - subtract;
	this->star->position = this->star->position - subtract;
	for (std::list<Planet *>::iterator i = planets.begin(); i != planets.end(); ++i) {
		Planet *p = *i;
		p->position = p->position - subtract;
	}
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
	Planet *p;
	for (std::list<Planet *>::iterator i = planets.begin(); i != planets.end(); ++i) {
		p = *i;
		p->draw();
	}

	// debug: draw line from planet to sun
	glBegin(GL_LINES);
	glVertex3f(star->position.x(), star->position.y(), star->position.z());
	glVertex3f(p->position.x(), p->position.y(), p->position.z());
	glEnd();
}
