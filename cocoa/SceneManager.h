#ifndef _TWS_SCENEMANAGER
#define _TWS_SCENEMANAGER

#include <GL/glfw.h>

namespace SPB {
	class SceneManager {
	private:
		// keys, etc
		double hfov;
		double vfov;
		double hsize;
		double vsize;
		double neardist;
		double fardist;

		void draw();
		void processKeys();
		void processEvents();
		void processClicks();

	public:
		SceneManager();
		~SceneManager();

		bool init();
		void step();
	};
}

#endif