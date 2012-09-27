#ifndef _SPB_NODE
#define _SPB_NODE

#include <Quaternion.h>

namespace SPB
{
	class Node {
	private:
		Vector3d _position, _velocity;
		Quatd _rotation;

	public:
		Node();

		Vector3d position();
        Quatd rotation();

		void setPosition(Vector3d &newpos);
		void setVelocity(Vector3d &newvel);
		void step();
		void rotatex(double angle);
		void rotatey(double angle);
		void rotatez(double angle);
		void moveforward(double distance, bool accelerate);
		void moveupward(double distance, bool accelerate);
		void straferight(double distance, bool accelerate);
		void stop();
        void follownode(Node *node);
	};
};

#endif
