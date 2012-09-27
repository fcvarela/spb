#include <Factory.h>

namespace SPB {
    Camera::Camera() {
        Vector3d initialVelocity = Vector3d(0.0, 0.0, 0.0) * 7706.6;
        Vector3d initialPosition = Vector3d(0.0, 0.0, 1.0) * (12000000.0);

        setPosition(initialPosition);
        setVelocity(initialVelocity);
    }

    void Camera::setPerspective() {
        GLdouble mat[16];

        rotation().glMatrix(mat);
        glMultMatrixd(mat);
        glTranslatef(-position().x(), -position().y(), -position().z());

        calculateFrustum();
    }
}
