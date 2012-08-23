/* noise functions over R2 & R3, implemented by pseudorandom tricubic spline */
/* (the Ken-ventional method) */
/* Ken Perlin 12/89 */
/* Ken Musgrave 5/90 */

#include "cloud.h"

	/* delta for derivative determination, and its inverse */
#define DELTA		0.001
#define DELTA_INV	1000.

#define B	0x100
#define BM	0xff

#define N	0x100000
#define NP	12   /* 2^N */
#define NM	0xfff

static p[B + B + 2];
static double g[B + B + 2][3];

	/* cubic spline interpolation */
#define s_curve(t)	( t * t * (3. - 2. * t) )
	/* linear interpolation */
#define lerp(t, a, b)	( a + t * (b - a) )
#define at3(rx,ry,rz)	( rx * q[0] + ry * q[1] + rz * q[2] )
#define at2(rx,ry)	( rx * q[0] + ry * q[1] )
#define setup(u,b0,b1,r0,r1)\
	t = u + N;\
	b0 = ((int)t) & BM;\
	b1 = (b0+1) & BM;\
	r0 = t - (int)t;\
	r1 = r0 - 1.;


double
Noise3(vec)
Vector vec;
{
	int bx0, bx1, by0, by1, bz0, bz1, b00, b10, b01, b11;
	double rx0, rx1, ry0, ry1, rz0, rz1, *q, sx, sy, sz, t;
	double a, b, c, d, u, v;
	register i, j;

#ifdef UNLIMITED_NOISE_DOMAIN
	filter_args(&vec.x, &vec.y, &vec.z);
#endif

	setup(vec.x, bx0,bx1, rx0,rx1);
	setup(vec.y, by0,by1, ry0,ry1);
	setup(vec.z, bz0,bz1, rz0,rz1);

	i = p[ bx0 ];
	j = p[ bx1 ];

	b00 = p[ i + by0 ];
	b10 = p[ j + by0 ];
	b01 = p[ i + by1 ];
	b11 = p[ j + by1 ];

	sx = s_curve(rx0);
	sy = s_curve(ry0);
	sz = s_curve(rz0);

	q = g[ b00 + bz0 ];	u = at3(rx0,ry0,rz0);
	q = g[ b10 + bz0 ];	v = at3(rx1,ry0,rz0);
	a = lerp(sx, u, v);

	q = g[ b01 + bz0 ];	u = at3(rx0,ry1,rz0);
	q = g[ b11 + bz0 ];	v = at3(rx1,ry1,rz0);
	b = lerp(sx, u, v);

	c = lerp(sy, a, b);		/* interpolate in y at lo z */

	q = g[ b00 + bz1 ];	u = at3(rx0,ry0,rz1);
	q = g[ b10 + bz1 ];	v = at3(rx1,ry0,rz1);
	a = lerp(sx, u, v);

	q = g[ b01 + bz1 ];	u = at3(rx0,ry1,rz1);
	q = g[ b11 + bz1 ];	v = at3(rx1,ry1,rz1);
	b = lerp(sx, u, v);

	d = lerp(sy, a, b);		/* interpolate in y at hi z */

	return 1.5 * lerp(sz, c, d);	/* interpolate in z */
} /* Noise3() */


double
Noise2(vec)
Vector	vec;
{
	int	bx0, bx1, by0, by1, b00, b10, b01, b11;
	double	rx0, rx1, ry0, ry1, *q, sx, sy, a, b, t, u, v, result;
	int i, j;

#ifdef UNLIMITED_NOISE_DOMAIN
	filter_args(&vec.x, &vec.y, &vec.z);
#endif

	setup(vec.x, bx0,bx1, rx0,rx1);
	setup(vec.y, by0,by1, ry0,ry1);

	i = p[ bx0 ];
	j = p[ bx1 ];

	b00 = p[ i + by0 ];
	b10 = p[ j + by0 ];
	b01 = p[ i + by1 ];
	b11 = p[ j + by1 ];

	sx = s_curve(rx0);
	sy = s_curve(ry0);

	q = g[ b00 ];		/* get random gradient */
	u = at2(rx0,ry0);	/* get weight on lo x side (lo y) */
	q = g[ b10 ];
	v = at2(rx1,ry0);	/* get weight on hi x side (lo y) */
	a = lerp(sx, u, v);	/* get value at distance sx between u & v */

				/* similarly at hi y... */
	q = g[ b01 ];	u = at2(rx0,ry1);
	q = g[ b11 ];	v = at2(rx1,ry1);
	b = lerp(sx, u, v);

	result = 1.5 * lerp(sy, a, b);	/* interpolate in y */

	return (result);
}  /* Noise2() */


#include "noise_table.c"

void
Init_Noise()
{
	int i;

        for (i= 0; i < B+B+2; ++i)
          {
              p[i]= p_precomputed[i];
              g[i][0]= g_precomputed[i][0];
              g[i][1]= g_precomputed[i][1];
              g[i][2]= g_precomputed[i][2];
          }
}
