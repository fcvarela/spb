#include <OpenGL/GL.h>
#include <math.h>

typedef struct {
    GLfloat A, B, C, D;
} plane_t;

union frustum_t {
    struct {plane_t t, b, l, r, n, f;};
    plane_t planes[6];
} frustum_t;

void extractPlane(plane_t *plane, GLfloat *mat, int row);
void calculateFrustum();
int sphereInFrustum(double *sphere);
int boxInFrustum(double *boundingBox);
double veclen(double *vec);
void geocentricToCarthesian(double *position, float latitude, float longitude);

union frustum_t globalFrustum;

void geocentricToCarthesian(double *position, float latitude, float longitude) {
    double lat = latitude;
    double lon = longitude;

    position[2] = cos(lon * 0.0174532925) * cos(lat * 0.0174532925);
    position[0] = sin(lon * 0.0174532925) * cos(lat * 0.0174532925);
    position[1] = sin(lat * 0.0174532925);
}

void extractPlane(plane_t *plane, GLfloat *mat, int row) {
    int scale = (row < 0) ? -1 : 1;
    row = abs(row) - 1;

    // calculate plane coefficients from the matrix
    plane->A = mat[3] + scale * mat[row];
    plane->B = mat[7] + scale * mat[row + 4];
    plane->C = mat[11] + scale * mat[row + 8];
    plane->D = mat[15] + scale * mat[row + 12];

    // normalize the plane
    double length = sqrtf(plane->A * plane->A + plane->B * plane->B + plane->C * plane->C);
    plane->A /= length;
    plane->B /= length;
    plane->C /= length;
    plane->D /= length;
}

    // determines the current view frustum
void calculateFrustum() {
    // get the projection and modelview matrices
    GLfloat projection[16];
    GLfloat modelview[16];

    glGetFloatv(GL_PROJECTION_MATRIX, projection);
    glGetFloatv(GL_MODELVIEW_MATRIX, modelview);

    // use OpenGL to multiply them
    glPushMatrix();
    glLoadMatrixf(projection);
    glMultMatrixf(modelview);
    glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
    glPopMatrix();

    // extract each plane
    extractPlane(&globalFrustum.l, modelview, 1);
    extractPlane(&globalFrustum.r, modelview, -1);
    extractPlane(&globalFrustum.b, modelview, 2);
    extractPlane(&globalFrustum.t, modelview, -2);
    extractPlane(&globalFrustum.n, modelview, 3);
    extractPlane(&globalFrustum.f, modelview, -3);
}

int sphereInFrustum(double *sphere) {
    double dist;
    int i;
    for (i=0; i<6; i++) {
        dist = globalFrustum.planes[i].A * sphere[0] +
            globalFrustum.planes[i].B * sphere[1] +
            globalFrustum.planes[i].C * sphere[2] +
            globalFrustum.planes[i].D;
        if (dist < -sphere[3])
            return 0;

        if (fabs(dist) < sphere[3])
            return 1;
    }
    return 1;
}

double veclen(double *vec) {
    return sqrt(vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2]);
}

int boxInFrustum(double *boundingBox) {
    double dist;

    // calculate camera distance to frustum
    uint8_t i, k;
    uint8_t in, out;
    for (i=0; i<6; i++) {
        in = out = 0;
        for (k=0; k<8 && (in==0 || out==0); k++) {
            dist = 
            globalFrustum.planes[i].A * boundingBox[k*3+0] +
            globalFrustum.planes[i].B * boundingBox[k*3+1] +
            globalFrustum.planes[i].C * boundingBox[k*3+2] +
            globalFrustum.planes[i].D;

            if (dist < 0) out++;
            else in++;
        }
        if (!in)
            return 0;
        else if (out)
            return 1;
    }

    return 1;
}
