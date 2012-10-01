#include <iostream>
#include <Common.h>
#include <GameSceneManager.h>

// provision a singleton
static GameSceneManager *gGameSceneManager;
GameSceneManager *getGameSceneManager() {
	if (!gGameSceneManager)
		gGameSceneManager = new GameSceneManager();

	return gGameSceneManager;
}

GameSceneManager::GameSceneManager() {
	std::cerr << "GameSceneManager alloc" << std::endl;
}

GameSceneManager::~GameSceneManager() {
	std::cerr << "GameSceneManager cleanup" << std::endl;
}

// and the rest
bool GameSceneManager::init() {
	std::cerr << "GameSceneManager init" << std::endl;

	// gl initialization
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	// lighting
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	// read config and set initial camera position
	this->camera = new Camera();

	// initialize the first star system. should come from config later
	StarSystem *firstStarSystem = new StarSystem();
	this->starSystems.push_front(firstStarSystem);

	return true;
}

void GameSceneManager::reshape() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, __width__, __height__);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void GameSceneManager::step() {
	// reposition camera
	this->camera->setPerspective();

	// update the view frustum
	calculateFrustum(this->frustum);

	// draw!
	for (std::list<StarSystem *>::iterator i = starSystems.begin(); i != starSystems.end(); ++i) {
		StarSystem *ss = *i;
		ss->step();
		ss->draw();
	}
}