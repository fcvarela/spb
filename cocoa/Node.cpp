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

void Node::rotatex(double angle) {
	Quatd nrot(Vector3d(1.0, 0.0, 0.0), angle);
	rotation = rotation * nrot;
}

void Node::rotatey(double angle) {
	Quatd nrot(Vector3d(0.0, 1.0, 0.0), angle);
	rotation = rotation * nrot;
}

void Node::rotatez(double angle) {
	Quatd nrot(Vector3d(0.0, 0.0, 1.0), angle);
	rotation = rotation * nrot;
}

void Node::moveforward(double distance) {
	Vector3d delta = -Vector3d(0.0, 0.0, -1.0) * distance * __dt__;
	rotation.rotate(delta);
	velocity += delta;
}

void Node::moveupward(double distance) {
	Vector3d delta = Vector3d(0.0, 1.0, 0.0) * distance * __dt__;
	rotation.rotate(delta);
	velocity += delta;
}

void Node::straferight(double distance) {
	Vector3d delta = Vector3d(1.0, 0.0, 0.0) * distance * __dt__;
	rotation.rotate(delta);
	velocity += delta;
}

void Node::stop() {
	acceleration = 0.0;
	velocity = 0.0;
}
