#ifndef __GAMESCENEMANAGER
#define __GAMESCENEMANAGER

#include <list>
#include <StarSystem.h>
#include <Camera.h>
#include <Common.h>
#include <Galaxy.h>

class GameSceneManager {
public:
	// list of star systems that exist
	std::list<StarSystem *> starSystems;
	frustum_t frustum;

	Camera *camera;
	Galaxy *galaxy;

	GameSceneManager();
	~GameSceneManager();

	bool init();
	void reshape();
	void step();
	void draw();
	void drawDebug();
	Node *nearestNode();
	void recalculatePositions(Vector3d &subtract);
};

GameSceneManager *getGameSceneManager();

#endif