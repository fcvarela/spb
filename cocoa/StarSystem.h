#ifndef __STARSYSTEM
#define __STARSYSTEM

#include <Node.h>

class StarSystem : public Node {
public:
	StarSystem();
	~StarSystem();

	void step();
	void draw();
};

#endif