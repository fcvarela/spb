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

	// read config and set initial camera position
	this->camera = new Camera();

	// initialize the first star system. should come from config later
	StarSystem *firstStarSystem = new StarSystem();
	firstStarSystem->label = std::string("SOLAR SYSTEM");

	StarSystem *secondStarSystem = new StarSystem();
	secondStarSystem->label = std::string("CENTAURI SYSTEM");
	secondStarSystem->position = Vector3d(0.0, 0.0, -1.0) * 4.366 * 9.4607E15;

	this->starSystems.push_front(firstStarSystem);
	this->starSystems.push_front(secondStarSystem);

	// set camera position to middle ground between star and planet
	Star *sun = firstStarSystem->star;
	sun->label = std::string("SUN");

	Star *centauri = secondStarSystem->star;
	centauri->label = std::string("ALPHA CENTAURI");

	camera->position = Vector3d(0.0, 0.0, sun->radius * 2.0);

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
	gluPerspective(__vfov__, __aratio__, near, far);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

// get planet closest to camera
Node *GameSceneManager::nearestNode() {
	std::list<StarSystem *>::iterator i = starSystems.begin();
	StarSystem *nearestSystem = *i;
	double mindistance = (camera->position - (*i)->position).length();
	double distance;

	for (i = starSystems.begin(); i != starSystems.end(); ++i) {
		distance = (camera->position - (*i)->position).length();
		if (distance < mindistance) {
			mindistance = distance;
			nearestSystem = (*i);
		}
	}

	// got closest system. get closest planet in system
	Node *nearestNode = nearestSystem->nearestNode(camera);

	return nearestNode;
}

// subtract node position from all nodes
void GameSceneManager::recalculatePositions(Vector3d &subtract) {
	for (std::list<StarSystem *>::iterator i = starSystems.begin(); i != starSystems.end(); ++i)
		(*i)->recalculatePositions(subtract);
}

void GameSceneManager::step() {
	Node *nearest = nearestNode();
	Vector3d nearest_position = nearest->position;
	double distance = (camera->position - nearest_position).length();

	double near = 1.0, far;
	far = distance * 3.0;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, __width__, __height__);
	gluPerspective(__vfov__, __aratio__, near, far);
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

	// set camera delta according to nearest node
	camera->position -= nearest_position;
	recalculatePositions(nearest_position);
	__camdelta__ = (nearest_position - camera->position).length()/3.0;
}
