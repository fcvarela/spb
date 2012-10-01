#ifndef __COMMON
#define __COMMON

#include <stdint.h>
#include <GL/glfw.h>

extern double __dt__;
extern double __lasttime__;
extern uint16_t __width__;
extern uint16_t __height__;

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