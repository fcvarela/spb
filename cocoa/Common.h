#ifndef __COMMON
#define __COMMON

#include <stdint.h>
#include <GL/glfw.h>

extern double __dt__;
extern double __lasttime__;
extern uint16_t __width__;
extern uint16_t __height__;
extern double __hfov__;
extern double __vfov__;
extern double __aratio__;
extern uint8_t __keys__[512];
extern double __camdelta__;

struct plane_t {GLfloat A, B, C, D;};
union frustum_t {
	struct {plane_t t, b, l, r, n, f;};
	plane_t planes[6];
};

// frustum tools
void calculateFrustum(frustum_t &frustum);
void extractPlane(plane_t &plane, GLfloat *mat, int row);
void globalStep();
void calcFOV();

void GLFWCALL My_Key_Callback(int key, int action);

#endif