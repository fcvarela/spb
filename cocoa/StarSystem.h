#ifndef __STARSYSTEM
#define __STARSYSTEM

#include <list>
#include <Node.h>
#include <Star.h>
#include <Planet.h>

class StarSystem : public Node {
public:
	StarSystem();
	~StarSystem();

	Star *star;
	std::list<Planet *> planets;

	void step();

	void draw();
};

#endif