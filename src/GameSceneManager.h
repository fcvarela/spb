#ifndef __GAMESCENEMANAGER
#define __GAMESCENEMANAGER

#include <list>
#include <StarSystem.h>
#include <Camera.h>
#include <Common.h>
#include <Galaxy.h>

class GameSceneManager {
public:
	frustum_t frustum;

	Camera *camera;
	Galaxy *galaxy;

	GameSceneManager();
	~GameSceneManager();

	bool init();
	void reshape();
	void step();
	void draw();
	void drawGalaxy();
	void drawSystems();
	void drawDebug();
};

GameSceneManager *getGameSceneManager();

#endif