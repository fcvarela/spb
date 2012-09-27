#ifndef _SPB_SCENEMANAGER
#define _SPB_SCENEMANAGER

#include <SceneManager.h>
#include <Camera.h>

namespace SPB {
	// singletons
	static SceneManager *gSceneManager;
	static Camera *gCamera;

	struct plane_t {
        GLfloat A, B, C, D;
    };

    union frustum_t {
        struct {
            plane_t t, b, l, r, n, f;
        };
        plane_t planes[6];
    };

    // global vars
	extern frustum_t _globalFrustum;
	extern double _dt;
	extern double _lastframetime;
	extern double _far;
	
	void extractPlane(plane_t &plane, GLfloat *mat, int row);
    void calculateFrustum();

	SceneManager *getSceneManager();
	Camera *getCamera();

	void updateDT();
}

#endif