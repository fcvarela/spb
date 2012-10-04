#include <Common.h>
#include <StarSystem.h>

#include <Star.h>
#include <Planet.h>

StarSystem::StarSystem(const libconfig::Setting &system) {
	system.lookupValue("name", this->label);

	this->position = Vector3d(system["position"][0], system["position"][1], system["position"][2]);
	std::cerr << "Star system " << this->label << " initialized at " << this->position << std::endl;

	this->star = new Star(system["star"], this);

	libconfig::Setting &planets = system["planets"];
	for (int i=0; i<planets.getLength(); i++) {
		Planet *newPlanet = new Planet(planets[i], this, NULL);
		this->planets.push_back(newPlanet);

		// does it have moons?
		bool got_moons = planets[i].exists("moons");
		if (got_moons) {
			libconfig::Setting &moons = planets[i]["moons"];
			for (int i=0; i<moons.getLength(); i++) {
				Planet *newMoon = new Planet(moons[i], this, newPlanet);
				newPlanet->moons.push_back(newMoon);
			}
		}
	}
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
