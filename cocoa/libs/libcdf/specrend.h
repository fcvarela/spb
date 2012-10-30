#ifndef _SPECREND_H
#define _SPECREND_H

/* A colour system is defined by the CIE x and y coordinates of
	its three primary illuminants and the x and y coordinates of
	the white point. */

struct colourSystem {
	const char *name;
	double xRed, yRed, xGreen, yGreen, xBlue, yBlue, xWhite, yWhite, gamma;
};

extern colourSystem NTSCsystem;
extern colourSystem EBUsystem;
extern colourSystem SMPTEsystem;
extern colourSystem CIEsystem;
extern colourSystem Rec709system;
extern double bbTemp;

void spectrum_to_xyz(double (*spec_intens)(double wavelength), double *x, double *y, double *z);
extern void xyz_to_rgb(struct colourSystem *cs, double xc, double yc, double zc, double *r, double *g, double *b);
extern double bb_spectrum(double wavelength);
extern void norm_rgb(double *r, double *g, double *b);
extern double bb_spectrum(double wavelength);

#endif
