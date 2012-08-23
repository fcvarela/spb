#include <stdio.h>
#include <math.h>
#include "cloud.h"


/*
 * The QAEB procedural ray tracing routine.
 *
 * Fractal functions module.
 *
 * This is mostly bunch of C code for terrain models that appears in the book 
 * "Textures and Modeling: A Procedural Approach," Ebert et al, Academic Press,
 * 1998.
 *
 * Copyright 1998 F. Kenton Musgrave
 * All rights reserved
 */

extern CameraType	camera;
extern OptionsType      options;
extern double		octaves_scalar;
extern double		Noise2();
extern double		Noise3();


/* 
 * Ridged multifractal - FKM 4/94
 *
 * Some good parameter values to start with:
 *
 *     H:            0.9
 *     offset:       1
 *     gain:         2
 */
double 
RidgedMultifractal( Vector point, double H, double lacunarity,
		    double octaves, double offset, double threshold )
{
	double  	result, frequency, signal, weight, Noise2();
	int             i;
	static          first = TRUE;
	static double   exponent_array[MAX_OCTAVES];

	/* precompute and store spectral weights */
	if ( first ) {

		/* compute weight for each frequency */
		for (i=0; i<MAX_OCTAVES; i++) 
			exponent_array[i] = pow( lacunarity, -i*H );
		
		first = FALSE;
	}

	/* get first octave */
	signal = Noise2( point );
	/* get absolute value of signal (this creates the ridges) */
	if ( signal < 0.0 ) signal = -signal;
	/* invert and translate (note that "offset" should be ~= 1.0) */
	signal = offset - signal;
	/* square the signal, to increase "sharpness" of ridges */
	signal *= signal;
	/* assign initial values */
	result = signal;
	weight = 1.0;

	for( i=1; weight>0.001 && i<octaves; i++ ) {
		point.x *= lacunarity;
		point.y *= lacunarity;
		point.z *= lacunarity;

		/* weight successive contributions by previous signal */
		weight = signal * threshold;
		if ( weight > 1.0 ) weight = 1.0;
		if ( weight < 0.0 ) weight = 0.0;
		signal = Noise2( point );
		if ( signal < 0.0 ) signal = -signal;
		signal = offset - signal;
		signal *= signal;
		/* weight the contribution */
		signal *= weight;
		result += signal * exponent_array[i];
	}
	
	return( result );

} /* RidgedMultifractal() */


/* 
 * Stats-by-Position multifractal - FKM 4/94
 *
 * Some good parameter values to start with:
 *
 *     H:            0.25
 *     offset:       .7
 *     gain:         1
 */
double 
HybridMultifractal( Vector point, double H, double lacunarity, 
		    double octaves, double offset, double gain )
{
	double		frequency, value, signal, weight, remainder, Noise2();
	int		i;
	static		first = TRUE;
	static double   exponent_array[MAX_OCTAVES];

	/* precompute and store spectral weights */
	if ( first ) {

		/* compute weight for each frequency */
		for (i=0; i<MAX_OCTAVES; i++) 
			exponent_array[i] = pow( lacunarity, -i*H );
		
		first = FALSE;
	}

	/* get first octave of function; later octaves are weighted */
	value = Noise2( point ) + offset;
	weight = gain * value;
	point.x *= lacunarity;
	point.y *= lacunarity;
	point.z *= lacunarity;

	/* inner loop of spectral construction, where the fractal is built */
	for (i=1; weight>0.001 && i<octaves; i++) {
		/* prevent divergence */
		if ( weight > 1.0 )  weight = 1.0;

		/* get next higher frequency */
		signal = ( Noise2( point ) + offset ) * exponent_array[i];
		/* add it in, weighted by previous freq's local value */
		value += weight * signal;
		/* update the (monotonically decreasing) weighting value */
		weight *= gain * signal;

		point.x *= lacunarity;
		point.y *= lacunarity;
		point.z *= lacunarity;
	} /* for */

	/* take care of remainder in "octaves" */
	remainder = octaves - (int)octaves;
	if ( remainder )
		/* "i" and spatial freq. are preset in loop above */
		value += remainder * Noise2( point ) * exponent_array[i];

	return( value );

} /* HybridMultifractal() */



/*
 * Heterogeneous procedural terrain fucntion: stats by altitude method.
 * Evaluated at "point"; returns value stored in "value".
 * 
 * Parameters:
 * 	"H" is the fractal increment
 * 	"lacunarity" is the gap between successive frequencies
 * 	"octaves" is the number of frequencies in the fBm
 * 	"offset" raises the terrain from ‘sea level’
 */
double
Hetero_Terrain( Vector point,
		double H, double lacunarity, double octaves, double offset )
{
	double		value, increment, frequency, remainder, Noise2();
	int			i;
	static		first = TRUE;
	static double   exponent_array[MAX_OCTAVES];

	/* precompute and store spectral weights */
	if ( first ) {

		/* compute weight for each frequency */
		for (i=0; i<MAX_OCTAVES; i++) 
			exponent_array[i] = pow( lacunarity, -i*H );
		
		first = FALSE;
	}

	/* first unscaled octave of function; later octaves are scaled */
	value = offset + Noise2( point );
	point.x *= lacunarity;
	point.y *= lacunarity;
	point.z *= lacunarity;

	/* inner loop of spectral construction, where the fractal is built */
	for (i=1; i<octaves; i++) {
		/* obtain displaced noise value */
		increment = Noise2( point ) + offset;
		/* scale amplitude appropriately for this frequency */
		increment *= exponent_array[i];
		/* scale increment by current ‘altitude’ of fucntion */
		increment *= value;
		/* add increment to "value" */
		value += increment;
		point.x *= lacunarity;
		point.y *= lacunarity;
		point.z *= lacunarity;
	} /* for */

	/* take care of remainder in "octaves" */
	remainder = octaves - (int)octaves;
	if ( remainder ) {
		/* "i" and spatial freq. are preset in loop above */
		/* note that the main loop code is made shorter here */
		increment = (Noise2( point ) + offset) * exponent_array[i];
		value += remainder * increment * value;
	}

	return( value );

} /* Hetero_Terrain() */



/*
 * Procedural multifractal evaluated at "point"; 
 * returns value stored in "value".
 * 
 * Parameters:
 * 	H" determines the highest fractal dimension
 * 	"lacunarity" is gap between successive frequencies
 * 	"octaves" is the number of frequencies in the fBm
 * 	"scale" scales the basis function & determines multifractality
 */
double
multifractal( Vector point,
	      double H, double lacunarity, double octaves, double scale )
{
	double		value, frequency, remainder, Noise3();
	int		i;
	static		first = TRUE;
	static double   exponent_array[MAX_OCTAVES];

	/* precompute and store spectral weights */
	if ( first ) {

		/* compute weight for each frequency */
		for (i=0; i<MAX_OCTAVES; i++) 
			exponent_array[i] = pow( lacunarity, -i*H );
		
		first = FALSE;
	}

	value = 1.0;		/* initialize ""value"" */

	/* inner loop of spectral construction, where the fractal is built */
	for (i=0; i<octaves; i++) {
		value *= exponent_array[i] * Noise3( point ) + 1.0;
		point.x *= lacunarity;
		point.y *= lacunarity;
		point.z *= lacunarity;
	} /* for */

	/* take care of remainder in "octaves" */
	remainder = octaves - (int)octaves;
	if ( remainder )
		/* "i" and spatial freq. are preset in loop above */
		value += remainder * exponent_array[i]
			 * Noise3( point ) + 1.0;

	return value;

} /* multifractal() */


/*
 * Procedural fBm evaluated at "point"; returns value stored in "value".
 * 
 * Parameters:
 * 	"H" is the fractal increment
 * 	"lacunarity" is the gap between successive frequencies
 * 	"octaves" is the number of frequencies in the fBm
 */
double
fBm( Vector point, double H, double lacunarity, double octaves )
{
	double		value, frequency, remainder, Noise3();
	static double   exponent_array[MAX_OCTAVES];
	static int	first=TRUE;
	int		i;

	/* precompute and store spectral weights */
	if ( first ) {

		/* compute weight for each frequency */
		for (i=0; i<MAX_OCTAVES; i++) 
			exponent_array[i] = pow( lacunarity, -i*H );
		
		first = FALSE;
	}

	value = 0.0;		/* initialize "value" */

	/* inner loop of spectral construction, where the fractal is built */
	for (i=0; i<octaves; i++) {
		value += Noise3( point ) * exponent_array[i];
		point.x *= lacunarity;
		point.y *= lacunarity;
		point.z *= lacunarity;
	} /* for */

	/* take care of remainder in "octaves" */
	remainder = octaves - (int)octaves;
	if ( remainder )
		/* "i" and spatial freq. are preset in loop above */
		value += remainder * Noise3( point ) * exponent_array[i];

	return( value );

} /* fBm() */


/*
 * Procedural "turbulence" function: fBm constructed from abs( Noise() )
 * 
 * Parameters:
 * 	"H" is the fractal increment
 * 	"lacunarity" is the gap between successive frequencies
 * 	"octaves" is the number of frequencies in the fBm
 */
double
turbulence( Vector point, double H, double lacunarity, double octaves )
{
	double		temp, value, frequency, remainder, Noise3();
	static double   exponent_array[MAX_OCTAVES];
	static int	first=TRUE;
	int		i;

	/* precompute and store spectral weights */
	if ( first ) {

		/* compute weight for each frequency */
		for (i=0; i<MAX_OCTAVES; i++) 
			exponent_array[i] = pow( lacunarity, -i*H );
		
		first = FALSE;
	}

	value = 0.0;		/* initialize "value" */

	/* inner loop of spectral construction, where the fractal is built */
	for (i=0; i<octaves; i++) {
		temp = Noise3( point ) * exponent_array[i];
		if ( temp < 0.0 )
			temp = -temp;
		value += temp;
		point.x *= lacunarity;
		point.y *= lacunarity;
		point.z *= lacunarity;
	} /* for */

	/* take care of remainder in "octaves" */
	remainder = octaves - (int)octaves;
	if ( remainder ) {
		/* "i" and spatial freq. are preset in loop above */
		temp = remainder * Noise3( point ) * exponent_array[i];
		if ( temp < 0.0 )
			temp = -temp;
		value +=  temp;
	}

	return( value );

} /* turbulence() */

