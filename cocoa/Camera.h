#ifndef __CAMERA
#define __CAMERA

#include <Node.h>

class Camera : public Node {
public:
	Camera();
	~Camera();

	void setPerspective();
};

#endif
