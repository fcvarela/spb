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
	delete this->camera;
	delete this->galaxy;
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
	/*std::list<StarSystem *>::iterator i = starSystems.begin();
	StarSystem *nearestSystem = *i;
	Star *nearestStar = (Star *)nearestSystem->star;
	*/
	camera->position = Vector3d(0.0, 0.0, 40000.0);

	// initialize the galaxy as type Sa
	galaxy = new Galaxy(20000, 4000, 0.0004, 0.75, 1.0, 0.5, 200, 300, 150000);

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

	// reposition camera
	Vector3d curpos = camera->position;

	__gpu_mutex__.lock();
	camera->step();
	__camvelocity__ = (camera->position - curpos).length()/__dt__;
	__near__ = 1.0;
	__far__ = distance * 100000000000.0;

	// step
	for (std::list<StarSystem *>::iterator i = starSystems.begin(); i != starSystems.end(); ++i) {
		StarSystem *ss = *i;
		ss->step();
	}

	// set camera delta according to nearest node
	/*
	camera->position -= nearest_position;
	recalculatePositions(nearest_position);
	*/
	__gpu_mutex__.unlock();

	__camdelta__ = camera->position.length();
	if (__selectednode__ != NULL) {
		__camdelta__ = (__selectednode__->position - camera->position).length();
	}
}

void GameSceneManager::draw() {
	__gpu_mutex__.lock();
	CGLSetCurrentContext(__render_ctx__);
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

	// got clicks? draw color coded objects to figure out if there
	// was a hit
	if (__mousebuttons__[0] == 1) {
		galaxy->drawColored();
		GLubyte color[3];
		glReadPixels(__mousepos__[0], __height__ - 1 - __mousepos__[1], 1, 1, GL_RGB, GL_UNSIGNED_BYTE, color);

		// ids are sequencial
		uint32_t id = color[2] + color[1]*256 + color[0]*65536;
		if (id > 0) {
			// get first id
			uint32_t firstid = galaxy->m_pStars[0].colorid[2] +\
							   galaxy->m_pStars[0].colorid[1] * 256 +\
							   galaxy->m_pStars[0].colorid[0] * 65536;

			__selectednode__ = &galaxy->m_pStars[id-firstid];
		}

		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glfwPollEvents();
	}

	// draw galaxy
	glEnable(GL_POINT_SPRITE);
	galaxy->draw();
	glDisable(GL_POINT_SPRITE);

	if (__selectednode__ != NULL) {
		glColor3f(1.0, 0.0, 0.0);
		glBegin(GL_LINES);
		glVertex3f(__selectednode__->position.x(), __selectednode__->position.y(), __selectednode__->position.z());
		glVertex3f(0., 0., 0.);
		glEnd();
	}

	// draw system
	/*
	for (std::list<StarSystem *>::iterator i = starSystems.begin(); i != starSystems.end(); ++i) {
		StarSystem *ss = *i;
		ss->draw();
	}
	*/

	glfwPollEvents();
	drawDebug();

	double curtime = glfwGetTime();
	__fps__ = 1.0/(curtime - __lastframe__);
	__lastframe__ = curtime;
	__gpu_mutex__.unlock();
}

void GameSceneManager::drawDebug() {
	glDepthMask(GL_FALSE);

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

	GLfloat botleft[2] = {-__width__/2.0f+8.0f, -__height__/2.0f+08.0f};
	GLfloat topleft[2] = {-__width__/2.0f+8.0f, -__height__/2.0f+26.0f};
	GLfloat topright[2] = {__width__/2.0f-8.0f, -__height__/2.0f+26.0f};
	GLfloat botright[2] = {__width__/2.0f-8.0f, -__height__/2.0f+08.0f};

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

	Octree *node = this->galaxy->octree->nodeForPosition(camera->position);
	int count = node->items.size();

	sprintf(debug, "SPS: %.2f FPS: %.2f Camera velocity: %.2f Nearest node: %s Galaxy dist: %.2f Galaxy SMA: %.2f CurrentOctant: %lld OctantStars: %d", 
		1.0/__dt__,
		__fps__,
		__camvelocity__,
		nearestNode()->label.c_str(),
		(galaxy->m_pos - camera->position).length(),
		galaxy->m_radGalaxy, node->index, count);

	__font__->Render(debug);
	glDepthMask(GL_TRUE);
}
