#include <Factory.h>

namespace SPB {
	frustum_t _globalFrustum;
	double _dt;
	double _lastframetime;
	double _far;

	// return singletons
	SceneManager *getSceneManager() {
		if (!gSceneManager)
			gSceneManager = new SPB::SceneManager();

		return gSceneManager;
	}

	Camera *getCamera() {
		if (!gCamera)
			gCamera = new SPB::Camera();

		return gCamera;
	}

	void updateDT() {
		double now = glfwGetTime();
		_dt = now - _lastframetime;
		_lastframetime = now;
	}

	void extractPlane(plane_t &plane, GLfloat *mat, int row) {
		int scale = (row < 0) ? -1 : 1;
		row = abs(row) - 1;

		// calculate plane coefficients from the matrix
		plane.A = mat[3] + scale * mat[row];
		plane.B = mat[7] + scale * mat[row + 4];
		plane.C = mat[11] + scale * mat[row + 8];
		plane.D = mat[15] + scale * mat[row + 12];

		// normalize the plane
		double length = sqrtf(plane.A * plane.A + plane.B * plane.B + plane.C * plane.C);
		plane.A /= length;
		plane.B /= length;
		plane.C /= length;
		plane.D /= length;
	}

	// determines the current view frustum
	void calculateFrustum() {
		// get the projection and modelview matrices
		GLfloat projection[16];
		GLfloat modelview[16];

		glGetFloatv(GL_PROJECTION_MATRIX, projection);
		glGetFloatv(GL_MODELVIEW_MATRIX, modelview);

		// use OpenGL to multiply them
		glPushMatrix();
		glLoadMatrixf(projection);
		glMultMatrixf(modelview);
		glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
		glPopMatrix();

		// extract each plane
		extractPlane(_globalFrustum.l, modelview, 1);
		extractPlane(_globalFrustum.r, modelview, -1);
		extractPlane(_globalFrustum.b, modelview, 2);
		extractPlane(_globalFrustum.t, modelview, -2);
		extractPlane(_globalFrustum.n, modelview, 3);
		extractPlane(_globalFrustum.f, modelview, -3);
	}
}
