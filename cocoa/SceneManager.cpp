#include <Factory.h>

namespace SPB {
	// ctor
	SceneManager::SceneManager() {
		this->hfov = 0.0;
		this->vfov = 0.0;
		this->hsize = 0.0;
		this->vsize = 0.0;
		this->neardist = 0.0;
		this->fardist = 0.0;
	}

	// dtor
	SceneManager::~SceneManager() {}

	// initialize gl stuff
	bool SceneManager::init() {
		// specify shading model
		glShadeModel(GL_SMOOTH);

		// enable blending and specify how to blend
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// specify line antialias
		glEnable(GL_LINE_SMOOTH);
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

		// clear color
		glClearColor(0.0, 0.0, 0.0, 1.0);

		// line and point width
		glLineWidth(1.0);
		glPointSize(1.0);

		// specify culling method and enable
		glCullFace(GL_BACK);
		glEnable(GL_CULL_FACE);

		// perspective correction
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

		// wireframe for debug (GL_LINE)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glPolygonMode(GL_FRONT, GL_FILL);

		// depth testing
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_DEPTH_TEST);
		glClearDepth(1.0);

		return true;
	}

	void SceneManager::step() {
		updateDT();

		// runloop step
		processEvents();
		processClicks();
		processKeys();

		draw();
	}


	// parse user interaction & events
	void SceneManager::processKeys() {}
	void SceneManager::processEvents() {}
	void SceneManager::processClicks() {}

	// draw the scene
	void SceneManager::draw() {}
}