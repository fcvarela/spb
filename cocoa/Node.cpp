#include <Common.h>
#include <Node.h>

Node::Node() {
	rotation = Quatd(1.0, 0.0, 0.0, 0.0);
	position = Vector3d(0.0, 0.0, 0.0);
}

void Node::step() {}