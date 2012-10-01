#ifndef __GAMESCENEMANAGER
#define __GAMESCENEMANAGER

#include <list>
#include <StarSystem.h>
#include <Camera.h>
#include <Common.h>

class GameSceneManager {
public:
	// list of star systems that exist
	std::list<StarSystem *> starSystems;
	frustum_t frustum;

	Camera *camera;

	GameSceneManager();
	~GameSceneManager();

	bool init();
	void reshape();
	void step();
};

GameSceneManager *getGameSceneManager();

#endif