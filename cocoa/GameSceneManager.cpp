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
	//glEnable(GL_LIGHTING);
	//glEnable(GL_LIGHT0);

	// read config and set initial camera position
	this->camera = new Camera();

	// initialize the first star system. should come from config later
	StarSystem *firstStarSystem = new StarSystem();
	this->starSystems.push_front(firstStarSystem);

	// set camera position to middle ground between star and planet
	Star *star = firstStarSystem->star;
	std::list<Planet *>::iterator planet = firstStarSystem->planets.begin();
	Vector3d cameraPos = star->position;
	camera->position = cameraPos;

	// prepare our viewport
	this->reshape();

	return true;
}

void GameSceneManager::reshape() {
	// prepare matrices
	double near = 1.0;
	double far = 10.0;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, __width__, __height__);
	glFrustum(.5, -.5, .5 * __aratio__, .5 * __aratio__, near, far);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void GameSceneManager::step() {
	// set camera position to middle ground between star and planet
	std::list<StarSystem *>::iterator ss = this->starSystems.begin();
	std::list<Planet *>::iterator planet = (*ss)->planets.begin();
	__camdelta__ = ((camera->position-(*planet)->position).length() - (*planet)->radius) / 5.0;

	double near = 1.0;
	double far = 100000000000000000.0;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, __width__, __height__);
	glFrustum(.5, -.5, .5 * __aratio__, .5 * __aratio__, near, far);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// reposition camera
	this->camera->step();
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
