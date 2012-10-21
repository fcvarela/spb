#ifndef __TERRAINLOADER
#define __TERRAINLOADER

#include <list>
#include <tinythread.h>

class TerrainQuadtree;

class TerrainLoader {
public:
	TerrainLoader();
	~TerrainLoader();

	void enqueue(TerrainQuadtree *node);
	TerrainQuadtree *dequeue();

	// load queue
	std::list<TerrainQuadtree *>loadQueue;

	// load queue mutex
	tthread::mutex loadQueueMutex;
};

TerrainLoader *getTerrainLoader();

#endif
