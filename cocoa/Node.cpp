#include <Common.h>
#include <Node.h>

Node::Node() {
	rotation = Quatd(1.0, 0.0, 0.0, 0.0);
	position = Vector3d(0.0, 0.0, 0.0);
	velocity = Vector3d(0.0, 0.0, 0.0);
	acceleration = Vector3d(0.0, 0.0, 0.0);
}

void Node::step() {
	position += velocity * __dt__;
	velocity += acceleration * __dt__;
}

void Node::stop(bool full) {
	acceleration = 0.0;
	velocity = 0.0;
}
