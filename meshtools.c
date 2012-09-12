#include <stdlib.h>
#include <stdint.h>
#include <math.h>

void build_quadtree_element(double *mesh, uint8_t size, uint8_t reverse, uint8_t border, double *center, double *dx, double *dy);

void build_quadtree_element(double *mesh, uint8_t size, uint8_t reverse, uint8_t border, double *center, double *dx, double *dy) {
    double u, v, coord[3], dxovergridsize[3], dyovergridsize[3], gridsizeover2, gsize, upos, vpos;

    // temp stuff
    gsize = size + 1 + (border * 2);
    dxovergridsize[0] = dx[0]/(double)size;
    dxovergridsize[1] = dx[1]/(double)size;
    dxovergridsize[2] = dx[2]/(double)size;

    dyovergridsize[0] = dy[0]/(double)size;
    dyovergridsize[1] = dy[1]/(double)size;
    dyovergridsize[2] = dy[2]/(double)size;

    gridsizeover2 = size/2;

    double length = 0.0;

    for (u=0; u<gsize; u++) {
        for (v=0; v<gsize; v++) {
            // build
            coord[0] = center[0] + dxovergridsize[0] * ((v-border)-gridsizeover2) + dyovergridsize[0] * (gridsizeover2 - (u-border));
            coord[1] = center[1] + dxovergridsize[1] * ((v-border)-gridsizeover2) + dyovergridsize[1] * (gridsizeover2 - (u-border));
            coord[2] = center[2] + dxovergridsize[2] * ((v-border)-gridsizeover2) + dyovergridsize[2] * (gridsizeover2 - (u-border));

            // normalize
            length = sqrt(coord[0]*coord[0] + coord[1]*coord[1] + coord[2]*coord[2]);
            coord[0] /= length;
            coord[1] /= length;
            coord[2] /= length;

            if (reverse == 1)
                upos = (gsize-1-u)*(gsize)*3.0;
            else
                upos = gsize * u * 3.0;
            vpos = v*3.0;

            mesh[(int16_t)upos + (int16_t)vpos + 0] = coord[0];
            mesh[(int16_t)upos + (int16_t)vpos + 1] = coord[1];
            mesh[(int16_t)upos + (int16_t)vpos + 2] = coord[2];
        }
    }
}
