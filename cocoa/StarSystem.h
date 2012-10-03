#ifndef __STARSYSTEM
#define __STARSYSTEM

#include <list>
#include <libconfig.h++>

#include <Node.h>
#include <Star.h>
#include <Planet.h>

class StarSystem : public Node {
public:
	StarSystem(const libconfig::Setting &system);
	~StarSystem();

	Star *star;
	std::list<Planet *> planets;

	void draw();
	void step();
	Node *nearestNode(Node *node);
	void recalculatePositions(Vector3d &subtract);
};

#endif