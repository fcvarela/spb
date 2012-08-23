/*
 * Declarations for QAEB ray-creeper.
 *
 * Copyright 1998 F. Kenton Musgrave
 * All rights reserved
 */

/* LIMITS ZOOM-IN CAPABILITY (should dump core if exceeded) */
#define MAX_OCTAVES	128

#define EGG_SHAPE
#define JITTERING

#define TRUE		1
#define FALSE		0
#define EPSILON		1e-6

#define Boolean	unsigned char

#define M_1_2PI		0.15915494309189533576
#define LOG2E           1.4426950408889634074
#define LOG2(x)         (LOG2E * log(x))		/* nat. log to log 2 */
#define DEGTORAD(x)	((x) *  0.01745329251994329576)	/* times pi/180 */
#define RADTODEG(x)	((x) * 57.29577951308232087684)	/* times 180/pi */
#define ABS(a)		((a)>=0 ? (a): -(a))
#define FABS(a)		((a)>=0. ? (a): -(a))

	/* vector normalization */
#define NORM(v)		{double length, norm;				\
			 length = sqrt(v->x*v->x+v->y*v->y+v->z*v->z);	\
			 norm = 1.0 / length;				\
			 v->x *= norm;					\
			 v->y *= norm;					\
			 v->z *= norm; }

	/* vector cross product */
#define CROSS(a,b,o)	(o)->x = (a.y * b.z) - (a.z * b.y);	\
			(o)->y = (a.z * b.x) - (a.x * b.z);	\
			(o)->z = (a.x * b.y) - (a.y * b.x);

	/* scalor multiplication of a 3-vector */
#define SMULT(s, v)	( (v).x*=(s), (v).y*=(s), (v).z*=(s), (v) )

	/* vector operations */
#define DOT(a,b)  	((a).x*(b).x + (a).y*(b).y + (a).z*(b).z)
#define VECADD(a,b,c)  	{(c)->x=(a).x+(b).x;	\
			 (c)->y=(a).y+(b).y;	\
			 (c)->z=(a).z+(b).z; }
#define VECSUB(a,b,c)  	{(c)->x = (a).x-(b).x;	\
			 (c)->y = (a).y-(b).y;	\
			 (c)->z = (a).z-(b).z; }
#define SCALARPROD(a,b,c)	\
			{(c)->x=(a)*(b).x;	\
			 (c)->y=(a)*(b).y;	\
			 (c)->z=(a)*(b).z; }
#define COLORSCALE(c1,s,c2)	\
			{(c2)->r=(s)*(c1).r;	\
			 (c2)->g=(s)*(c1).g;	\
			 (c2)->b=(s)*(c1).b; }
#define RAYPOS(ray,t,pos)	\
        		{(pos)->x = (ray)->origin.x + t*(ray)->dir.x;	\
			 (pos)->y = (ray)->origin.y + t*(ray)->dir.y;	\
			 (pos)->z = (ray)->origin.z + t*(ray)->dir.z; }


typedef struct {	/* catch-all type for 3-D vectors and potisions */
		double x;
		double y;
		double z;
} Vector;


typedef struct {	/* unquantized illumination values */
	double r;
	double g;
	double b;
} Color;


typedef struct {	/* quantized illumination values */
	unsigned char rc;
	unsigned char gc;
	unsigned char bc;
} ColorChar;


typedef struct {	/* ray struct */
        long    rayID;		/* for stats/etc */
        Vector	origin;		/* origin of ray */
        Vector	dir;		/* direction of ray (unit vector) */
        int	generation;	/* how many times ray has bounced around */
        int	shadow;		/* set to TRUE if it is a shadow ray */
} Ray;


typedef struct {	/* intersection data */
	double distance;	/* the distance to the intersection */
	Vector intersect;	/* intersection point */
	Vector normal;		/* the surface normal at the intersection */
} HitData;


typedef struct {	/* viewing parameters */
        Vector	eyep;		/* position of camera */
        Vector	look_pt;	/* where camera is pointed at */
        Vector	view_dir;	/* direction camera is pointed in */
        Vector	up_dir;		/* define "up" for camera */
        Vector	right_dir;	/* define "right" for camera */
        Vector	corner_ray;	/* the ray through pixel [0,0] */
	int	hres;		/* horizontal resolution */
	int	vres;		/* horizontal resolution */
        double	hfov;		/* horizontal field of view */
        double	vfov;           /* vertical field of view */
        double	pix_width;	/* pixel width at unit distance */
        double	pix_height;     /* pixel height at unit distance */
	double	near_clip;	/* near clipping distance */
	double	far_clip;	/* far clipping distance */
} CameraType;


typedef struct {	/* options data */
	double	step_scale;	/* scales the increments in ray-creeping */
	int	shadstep_scale;	/* scales the increments for shadow rays */
	double	gamma;		/* image gamma value */
	double	noise;		/* add noise to the image? */
	double  threshold;	/* crossover from air to cloud */
	double	density_scale;	/* scales cloud density */
	double	shad_clip_dist;	/* marching distance for shadow rays */
} OptionsType;


typedef struct {	/* light type */
	Vector	direction;	/* direction light travels in */
	Color	color;		/* color of light source */
} Light;


typedef struct {	/* plane type */
	Vector	position;	/* a point on the plane */
	Vector	normal;		/* surface normal of the plane */
	double	D;		/* D component of the plane equation */
} Plane;

