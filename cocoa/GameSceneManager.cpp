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
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

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
	camera->position = Vector3d(0.0, 0.0, nearestStar->radius * 10.0);

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
	__far__ = distance * 100000000000.0;

	// reposition camera
	Vector3d curpos = camera->position;
	camera->step();
	__camvelocity__ = (camera->position - curpos).length()/__dt__;

	// step
	__reposition_mutex__.lock();
	for (std::list<StarSystem *>::iterator i = starSystems.begin(); i != starSystems.end(); ++i) {
		StarSystem *ss = *i;
		ss->step();
	}

	// set camera delta according to nearest node
	camera->position -= nearest_position;
	recalculatePositions(nearest_position);
	__camdelta__ = (nearest_position - camera->position).length()/3.0;
	__reposition_mutex__.unlock();
}

void GameSceneManager::draw() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, __width__, __height__);
	gluPerspective(__vfov__, __aratio__, __near__, __far__);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// loclk
	__reposition_mutex__.lock();

	// set camera perspective
	this->camera->setPerspective();

	// update the view frustum
	calculateFrustum(this->frustum);

	// draw
	for (std::list<StarSystem *>::iterator i = starSystems.begin(); i != starSystems.end(); ++i) {
		StarSystem *ss = *i;
		ss->draw();
	}

	// unlock
	__reposition_mutex__.unlock();

	glDisable(GL_LIGHTING);
	drawDebug();
	glEnable(GL_LIGHTING);

	double curtime = glfwGetTime();
	__fps__ = 1.0/(curtime - __lastframe__);
	__lastframe__ = glfwGetTime();
}

void GameSceneManager::drawDebug() {
	// debug
	// set up an orthogonal 2d projection matrixf
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// assume width > height
	GLdouble size = __width__/2.0f;
	glOrtho(-size*__aratio__, size*__aratio__, -size, size, -1.0, 1.0);

	// set 1 scren unit = 1 pixel
	glScaled(__aratio__, __aratio__, 1.0);

	// reset to modelview
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	GLfloat botleft[2] = {-__width__/2.0f+8.0, -__height__/2.0f+08.0};
	GLfloat topleft[2] = {-__width__/2.0f+8.0, -__height__/2.0f+26.0};
	GLfloat topright[2] = {__width__/2.0f-8.0, -__height__/2.0f+26.0};
	GLfloat botright[2] = {__width__/2.0f-8.0, -__height__/2.0f+08.0};

	// draw background
	glColor4f(0.2, 0.2, 0.2, 0.4);
	glBegin(GL_QUADS);
	glVertex2fv(topright);
	glVertex2fv(topleft);
	glVertex2fv(botleft);
	glVertex2fv(botright);
	glEnd();

	glColor3f(1.0, 1.0, 1.0);
	glRasterPos2f(-__width__/2.0f+12.0, -__height__/2.0f+13.0);
	char debug[1024];
	sprintf(debug, "SPS: %.2f FPS: %.2f Camera velocity: (%.2f km/h, %.2f UA/s, %.2f c) Nearest node: %s", 
		1.0/__dt__,
		__fps__,
		__camvelocity__/3.6,
		__camvelocity__*6.68458712E-12,
		__camvelocity__/299792458.0,
		nearestNode()->label.c_str());

	__font__->Render(debug);
	glEnable(GL_DEPTH_TEST);
}
