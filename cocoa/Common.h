#ifndef __COMMON
#define __COMMON

#include <OpenGL/OpenGL.h>
#include <stdint.h>
#include <GL/glfw.h>
#include <FTGL/ftgl.h>
#include <tinythread.h>

extern double __dt__;
extern double __lasttime__;
extern double __lastframe__;
extern double __camvelocity__;
extern double __fps__;
extern uint16_t __width__;
extern uint16_t __height__;
extern double __hfov__;
extern double __vfov__;
extern double __aratio__;
extern double __near__;
extern double __far__;
extern uint8_t __keys__[512];
extern double __camdelta__;
extern FTFont *__font__;
extern int __running__;

extern CGLContextObj __procedural_gen_ctx__;
extern tthread::mutex __procedural_gen_mutex__;

struct plane_t {GLfloat A, B, C, D;};
union frustum_t {
	struct {plane_t t, b, l, r, n, f;};
	plane_t planes[6];
};

// frustum tools
void calculateFrustum(frustum_t &frustum);
void extractPlane(plane_t &plane, GLfloat *mat, int row);
void globalStep(void *arg);
void proceduralGenLoop(void *arg);
void calcFOV();

// input callbacks
void GLFWCALL My_Key_Callback(int key, int action);

#endif