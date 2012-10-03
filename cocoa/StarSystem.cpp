#include <Common.h>
#include <StarSystem.h>

StarSystem::StarSystem(const libconfig::Setting &system) {
	system.lookupValue("name", this->label);

	this->position = Vector3d(system["position"][0], system["position"][1], system["position"][2]);
	std::cerr << "Star system " << this->label << " initialized at " << this->position << std::endl;

	this->star = new Star(system["star"], this);

	libconfig::Setting &planets = system["planets"];
	for (int i=0; i<planets.getLength(); i++) {
		Planet *newPlanet = new Planet(system["planets"][i], this);
		this->planets.push_back(newPlanet);
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

		/*
		glBegin(GL_LINES);
		glVertex3f(star->position.x(), star->position.y(), star->position.z());
		glVertex3f(p->position.x(), p->position.y(), p->position.z());
		glEnd();
		*/
	}

	glDisable(GL_DEPTH_TEST);
	glColor3f(1.0, 0.0, 0.0);
	glRasterPos3f(position.x(), position.y(), position.z());
	__font__->Render(this->label.c_str());
	glEnable(GL_DEPTH_TEST);
}
