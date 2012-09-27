#include <Factory.h>

namespace SPB {
    Node::Node() {
        _rotation = Quatd(1.0, 0.0, 0.0, 0.0);
    }

    void Node::step() {
        // estimate position increment
        _position += _velocity * _dt;
    }

    void Node::rotatex(double angle) {
        Quaternion<double> nrot(Vector3d(1.0, 0.0, 0.0), angle);
        _rotation = _rotation * nrot;
    }

    void Node::rotatey(double angle) {
        Quaternion<double> nrot(Vector3d(0.0, 1.0, 0.0), angle);
        _rotation = _rotation * nrot;
    }

    void Node::rotatez(double angle) {
        Quaternion <double> nrot(Vector3d(0.0, 0.0, 1.0), angle);
        _rotation = _rotation * nrot;
    }

    void Node::moveforward(double distance, bool accelerate) {
        Vector3d delta = -Vector3d(0.0, 0.0, -1.0) * distance * (accelerate?_dt:1.0);
        _rotation.rotate(delta);
        _velocity += delta;
    }

    void Node::moveupward(double distance, bool accelerate) {
        Vector3d delta = Vector3d(0.0, 1.0, 0.0) * distance * (accelerate?_dt:1.0);
        _rotation.rotate(delta);
        _velocity += delta;
    }

    void Node::straferight(double distance, bool accelerate) {
        Vector3d delta = Vector3d(1.0, 0.0, 0.0) * distance * (accelerate?_dt:1.0);
        _rotation.rotate(delta);
        _velocity += delta;
    }

    void Node::follownode(Node *node) {
        _position = node->_position;
        _position.normalize();
        _position *= node->_position.length() + 2000.0;

        _velocity = node->_velocity;
    }

    void Node::stop() {_velocity = 0.0;}

    Vector3d Node::position() {return _position;}
    Quatd Node::rotation() {return _rotation;}

    void Node::setPosition(Vector3d &newpos) {
        _position = newpos;
        _velocity = Vector3d(0.0, 0.0, 0.0);
    }

    void Node::setVelocity(Vector3d &newvel) {_velocity = newvel;}
}
