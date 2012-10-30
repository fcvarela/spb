#include <iostream>
#include <TerrainLoader.h>

// provision a singleton
static TerrainLoader *gTerrainLoader;
TerrainLoader *getTerrainLoader() {
	if (!gTerrainLoader)
		gTerrainLoader = new TerrainLoader();

	return gTerrainLoader;
}

TerrainLoader::TerrainLoader() {}
TerrainLoader::~TerrainLoader() {}

void TerrainLoader::enqueue(TerrainQuadtree *node) {
	loadQueueMutex.lock();
	loadQueue.push_back(node);
	loadQueueMutex.unlock();
}

TerrainQuadtree *TerrainLoader::dequeue() {
	// lock the queue
	loadQueueMutex.lock();

	TerrainQuadtree *node = NULL;
	if (false == loadQueue.empty()) {
		// get ref to last node and remove it
		node = loadQueue.back();
		loadQueue.pop_back();
	}
	// unlock the list
	loadQueueMutex.unlock();

	return node;
}
