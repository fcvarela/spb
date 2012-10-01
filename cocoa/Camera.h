#ifndef __CAMERA
#define __CAMERA

#include <Node.h>

class Camera : public Node {
public:
	Camera();
	~Camera();

	void step();
	void setPerspective();

	void rotatex(double angle);
	void rotatey(double angle);
	void rotatez(double angle);
	void moveforward(double distance);
	void moveupward(double distance);
	void straferight(double distance);
};

#endif
