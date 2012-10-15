#ifndef __OCTREE
#define __OCTREE

#include <list>
#include <Vector.h>

class GallacticNode;

class Octree {
public:
	Octree(Octree *parent, uint64_t index, Vector3d &center, double size, unsigned short maxItemsPerNode);
	~Octree();

	Vector3d center;
	double size;
	unsigned short maxItemsPerNode;
	std::list<GallacticNode *> items;

	Octree *children[8], *parent;

	void insertItem(GallacticNode *node);
	void initChildren();
	bool containsPosition(Vector3d &position);
	Octree *nodeForPosition(Vector3d &position);

	uint64_t index;

	// debug
	double minx, miny, minz, maxx, maxy, maxz;
	unsigned long draw();
};

#endif