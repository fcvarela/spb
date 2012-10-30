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
void carthesianToGeocentric(double *latlon, double *position);
void normalize(double *v);

union frustum_t globalFrustum;

void geocentricToCarthesian(double *position, float latitude, float longitude) {
    double lat = latitude;
    double lon = longitude;

    position[2] = cos(lon * 0.0174532925) * cos(lat * 0.0174532925);
    position[0] = sin(lon * 0.0174532925) * cos(lat * 0.0174532925);
    position[1] = sin(lat * 0.0174532925);
}

void carthesianToGeocentric(double *latlon, double *position) {
    double longitude = atan2(position[0], position[2]) * 180.0/M_PI;
    double latitude = M_PI/2.0 - atan2(sqrt(position[2]*position[2] + position[0]*position[0]), position[1]);
    latitude *= 180.0/M_PI;
    latlon[0] = latitude;
    latlon[1] = longitude;
}

void normalize(double *v) {
    double length = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    v[0] /= length;
    v[1] /= length;
    v[2] /= length;
}

// determines the current view frustum
void calculateFrustum() {
    // get the projection and modelview matrices
    GLfloat projection[16];
    GLfloat modelview[16];
    GLfloat modelviewprojection[16];

    glGetFloatv(GL_PROJECTION_MATRIX, projection);
    glGetFloatv(GL_MODELVIEW_MATRIX, modelview);

    // use OpenGL to multiply them
    glPushMatrix();
    glLoadMatrixf(projection);
    glMultMatrixf(modelview);
    glGetFloatv(GL_MODELVIEW_MATRIX, modelviewprojection);
    glPopMatrix();

    // Left clipping plane
    globalFrustum.planes[0].A = modelviewprojection[3] +  modelviewprojection[0];
    globalFrustum.planes[0].B = modelviewprojection[7] +  modelviewprojection[4];
    globalFrustum.planes[0].C = modelviewprojection[11] + modelviewprojection[8];
    globalFrustum.planes[0].D = modelviewprojection[15] + modelviewprojection[12];

    // Right clipping plane
    globalFrustum.planes[1].A = modelviewprojection[3] -  modelviewprojection[0];
    globalFrustum.planes[1].B = modelviewprojection[7] -  modelviewprojection[4];
    globalFrustum.planes[1].C = modelviewprojection[11] - modelviewprojection[8];
    globalFrustum.planes[1].D = modelviewprojection[15] - modelviewprojection[12];

    // Top clipping plane
    globalFrustum.planes[2].A = modelviewprojection[3] -  modelviewprojection[1];
    globalFrustum.planes[2].B = modelviewprojection[7] -  modelviewprojection[5];
    globalFrustum.planes[2].C = modelviewprojection[11] - modelviewprojection[9];
    globalFrustum.planes[2].D = modelviewprojection[15] - modelviewprojection[13];

    // Bottom clipping plane
    globalFrustum.planes[3].A = modelviewprojection[3] +  modelviewprojection[1];
    globalFrustum.planes[3].B = modelviewprojection[7] +  modelviewprojection[5];
    globalFrustum.planes[3].C = modelviewprojection[11] + modelviewprojection[9];
    globalFrustum.planes[3].D = modelviewprojection[15] + modelviewprojection[13];

    // Near clipping plane
    globalFrustum.planes[4].A = modelviewprojection[3] +  modelviewprojection[2];
    globalFrustum.planes[4].B = modelviewprojection[7] +  modelviewprojection[6];
    globalFrustum.planes[4].C = modelviewprojection[11] + modelviewprojection[10];
    globalFrustum.planes[4].D = modelviewprojection[15] + modelviewprojection[14];

    // Far clipping plane
    globalFrustum.planes[5].A = modelviewprojection[3] -  modelviewprojection[2];
    globalFrustum.planes[5].B = modelviewprojection[7] -  modelviewprojection[6];
    globalFrustum.planes[5].C = modelviewprojection[11] - modelviewprojection[10];
    globalFrustum.planes[5].D = modelviewprojection[15] - modelviewprojection[14];

    int i;
    plane_t *plane;
    for (i=0; i<6; i++) {
        plane = &globalFrustum.planes[i];
        double length = sqrtf(plane->A * plane->A + plane->B * plane->B + plane->C * plane->C);
        plane->A /= length;
        plane->B /= length;
        plane->C /= length;
        plane->D /= length;
    }
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
