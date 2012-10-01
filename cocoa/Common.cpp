#include <iostream>

#include <GL/glfw.h>
#include <GameSceneManager.h>
#include <Common.h>

double __lasttime__ = glfwGetTime();
double __dt__ = 0.0;
uint16_t __width__ = 0;
uint16_t __height__ = 0;

void globalStep() {
	double now = glfwGetTime();
	__dt__ = now - __lasttime__;
	__lasttime__ = now;
}

// frustum tools
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
void calculateFrustum(frustum_t &frustum) {
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
	extractPlane(frustum.l, modelview, 1);
	extractPlane(frustum.r, modelview, -1);
	extractPlane(frustum.b, modelview, 2);
	extractPlane(frustum.t, modelview, -2);
	extractPlane(frustum.n, modelview, 3);
	extractPlane(frustum.f, modelview, -3);
}
