#include <libconfig.h++>
#include <iostream>

#include <Common.h>
#include <GameSceneManager.h>
#include <Star.h>

// provision a singleton
static GameSceneManager *gGameSceneManager;
GameSceneManager *getGameSceneManager() {
	if (!gGameSceneManager)
		gGameSceneManager = new GameSceneManager();

	return gGameSceneManager;
}

GameSceneManager::GameSceneManager() {
}

GameSceneManager::~GameSceneManager() {
}

// and the rest
bool GameSceneManager::init() {
	// gl initialization
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	// read config and set initial camera position
	this->camera = new Camera();

	// iterate systems in config and add them to list
	libconfig::Config cfg;
	try {
		cfg.readFile("conf/universe.cfg");
		const libconfig::Setting& root = cfg.getRoot();
		const libconfig::Setting &systems = root["systems"];
		for (int i=0; i<systems.getLength(); i++) {
			const libconfig::Setting &system = systems[i];
			StarSystem *newSystem = new StarSystem(system);
			starSystems.push_back(newSystem);
		}
	} catch(const libconfig::FileIOException &fioex) {
		std::cerr << "I/O error while reading file." << std::endl;
		return(EXIT_FAILURE);
	} catch(const libconfig::ParseException &pex) {
		std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine() << " - " << pex.getError() << std::endl;
		return(EXIT_FAILURE);
	}

	// set camera position to middle ground between star and planet
	std::list<StarSystem *>::iterator i = starSystems.begin();
	StarSystem *nearestSystem = *i;
	Star *nearestStar = (Star *)nearestSystem->star;
	camera->position = Vector3d(0.0, 0.0, nearestStar->radius * 2.0);

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

	__near__ = 1.0;
	__far__ = distance * 3.0;

	// reposition camera
	this->camera->step();

	// step
	for (std::list<StarSystem *>::iterator i = starSystems.begin(); i != starSystems.end(); ++i) {
		StarSystem *ss = *i;
		ss->step();
	}

	// set camera delta according to nearest node
	camera->position -= nearest_position;
	recalculatePositions(nearest_position);
	__camdelta__ = (nearest_position - camera->position).length()/3.0;
}

void GameSceneManager::draw() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, __width__, __height__);
	gluPerspective(__vfov__, __aratio__, __near__, __far__);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// set camera perspective
	this->camera->setPerspective();

	// update the view frustum
	calculateFrustum(this->frustum);

	// step
	for (std::list<StarSystem *>::iterator i = starSystems.begin(); i != starSystems.end(); ++i) {
		StarSystem *ss = *i;
		ss->draw();
	}
}
