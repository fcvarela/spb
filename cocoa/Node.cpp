#include <Common.h>
#include <Node.h>

Node::Node() {
	rotation = Quatd(1.0, 0.0, 0.0, 0.0);
	position = Vector3d(0.0, 0.0, 0.0);
}

void Node::step() {}

void Node::draw() {
	// debug label
	glColor3f(1.0, 1.0, 1.0);
	glRasterPos3f(position.x(), position.y(), position.z());
	__font__->Render(this->label.c_str());
}