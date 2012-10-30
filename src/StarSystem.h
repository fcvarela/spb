#ifndef __STARSYSTEM
#define __STARSYSTEM

#include <list>
#include <Galaxy.h>

class Star;
class Planet;

class StarSystem : public GallacticNode {
public:
	StarSystem();
	~StarSystem();

	Star *star;
	std::list<Planet *> planets;

	void draw();
	void step();
	Node *nearestNode(Node *node);
	void recalculatePositions(Vector3d &subtract);
};

#endif
