#ifndef __STARSYSTEM
#define __STARSYSTEM

#include <list>
#include <libconfig.h++>

#include <Node.h>

class Star;
class Planet;

class StarSystem : public Node {
public:
	StarSystem(uint32_t seed);
	~StarSystem();

	Star *star;
	std::list<Planet *> planets;

	void draw();
	void step();
	Node *nearestNode(Node *node);
	void recalculatePositions(Vector3d &subtract);
};

#endif