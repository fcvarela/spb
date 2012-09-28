#ifndef __COMMON
#define __COMMON

#include <GL/glfw.h>

extern double dt;
extern double lasttime;

struct plane_t {GLfloat A, B, C, D;};
union frustum_t {
	struct {plane_t t, b, l, r, n, f;};
	plane_t planes[6];
};

// frustum tools
void calculateFrustum(frustum_t &frustum);
void extractPlane(plane_t &plane, GLfloat *mat, int row);
void globalStep();

#endif