#include <Common.h>
#include <StarSystem.h>

#include <Star.h>
#include <Planet.h>

StarSystem::StarSystem(uint32_t seed) {
	
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

		for (std::list<Planet *>::iterator j = p->moons.begin(); j != p->moons.end(); ++j) {
			Planet *m = *j;
			dist = (m->position - node->position).length();
			if (dist < shortest_distance) {
				shortest_distance = dist;
				nearest = m;
			}
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
		for (std::list<Planet *>::iterator j = p->moons.begin(); j != p->moons.end(); j++) {
			Planet *m = *j;
			m->position = m->position - subtract;
		}
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

	Node::draw();
}
