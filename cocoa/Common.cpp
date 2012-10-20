#include <iostream>

#include <GL/glfw.h>
#include <GameSceneManager.h>
#include <TerrainLoader.h>
#include <TerrainQuadtree.h>
#include <Common.h>
#include <tinythread.h>

double __lasttime__ = glfwGetTime();
double __dt__ = 0.0;
double __lastframe__ = 0.0;
double __camvelocity__ = 0.0;
double __fps__ = 0.0;
double __aratio__ = 0.0;
double __hfov__ = 0.0;
double __vfov__ = 0.0;
double __near__ = 0.0;
double __far__ = 0.0;
uint16_t __width__ = 0;
uint16_t __height__ = 0;
uint8_t __keys__[512];
double __camdelta__ = 0.0;
int __running__ = GL_TRUE;

FTFont *__font__;

CGLContextObj __procedural_gen_ctx__;
CGLContextObj __render_ctx__;

tthread::mutex __gpu_mutex__;

void globalStep(void *arg) {
	while (__running__) {
		__gpu_mutex__.lock();
		double now = glfwGetTime();
		__dt__ = now - __lasttime__;
		__lasttime__ = now;
		for (uint16_t i=0; i<512; i++)
			__keys__[i] = glfwGetKey(i);
		getGameSceneManager()->step();
		__gpu_mutex__.unlock();
	}
}

void proceduralGenLoop(void *arg) {
	// we don't need vsync here...
	GLint sync = 0;
	CGLSetParameter(__procedural_gen_ctx__, kCGLCPSwapInterval, &sync);
	CGLSetCurrentContext(__procedural_gen_ctx__);

	while (__running__) {
		usleep(5000);
		__gpu_mutex__.lock();
		// set our context appropriately
		CGLSetCurrentContext(__procedural_gen_ctx__);
		TerrainLoader *loader = getTerrainLoader();
		TerrainQuadtree *node = loader->dequeue();
		if (node != NULL) {
			node->init();
		}
		CGLFlushDrawable(__procedural_gen_ctx__);
		__gpu_mutex__.unlock();
	}

	CGLDestroyContext(__procedural_gen_ctx__);
}

void calcFOV() {
	__aratio__ = (double)__width__/(double)__height__;
	__hfov__ = 60.0;
    __vfov__ = 2.0 * atan(tan(__hfov__/2.0 * M_PI/180.0) / __aratio__)*180.0/M_PI;
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

int boxInFrustum(double *boundingBox) {
	GameSceneManager *sm = getGameSceneManager();
	double dist;

	// calculate camera distance to frustum
	uint8_t i, k;
	uint8_t in, out;
	for (i=0; i<6; i++) {
		in = out = 0;
		for (k=0; k<8 && (in==0 || out==0); k++) {
			dist = 
			sm->frustum.planes[i].A * boundingBox[k*3+0] +
			sm->frustum.planes[i].B * boundingBox[k*3+1] +
			sm->frustum.planes[i].C * boundingBox[k*3+2] +
			sm->frustum.planes[i].D;

			if (dist < 0) out++;
			else in++;
		}
		if (!in)
			return 0;
		else if (out)
			return 1;
	}

	return 1;
}
