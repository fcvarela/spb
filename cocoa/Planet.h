#ifndef __PLANET
#define __PLANET

#include <Node.h>

class Planet : public Node {
public:
	Planet();
	~Planet();

	void draw();
};

#endif