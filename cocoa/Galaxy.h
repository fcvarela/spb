#ifndef __GALAXY
#define __GALAXY

#include <GL/glfw.h>
#include <Node.h>
#include <Octree.h>

struct Color {
	double r;
	double g;
	double b;
};

class GallacticNode : public Node {
public:
	GallacticNode();
	const Vector3d& CalcXY();

 	double m_theta;			// position on the ellipse
	double m_velTheta;		// angular velocity
	double m_angle;			// inclination of the ellipse
	double m_inclinationx;	// orbital inclination
	double m_inclinationy;	// orbital inclination
	double m_a;				// minor semiaxis
	double m_b;				// major semiaxis
	double m_temp;			// star temperature
	double m_mag;			// brigtness;
	Vector3d  m_center;		// center of the elliptical orbit
};

class Galaxy {
public:
	Galaxy(double rad, double radCore, double deltaAng, double ex1, double ex2, double sigma, double velInner, double velOuter, int numStars);
	~Galaxy();

	const Vector3d& GetStarPos(int idx);

	void drawStars();
	void drawDust();
	void drawH2();
	void draw();

	Color ColorFromTemperature(double temp) const;
	double GetOrbitalVelocity(double rad) const;
	double GetExcentricity(double r) const;
	void SingleTimeStep(double time);
	double GetAngularOffset(double rad) const;

	double my_random();

	int m_colNum;
	double m_t0, m_t1, m_dt;
	Color m_col[200];
	GLuint m_texStar;

	void InitStars(double sigma);
	
	double m_elEx1;          ///< Excentricity of the innermost ellipse
	double m_elEx2;          ///< Excentricity of the outermost ellipse
	double m_velOrigin;      ///< Velovity at the innermost core in km/s
	double m_velInner;       ///< Velocity at the core edge in km/s
	double m_velOuter;       ///< Velocity at the edge of the disk in km/s
	double m_angleOffset;    ///< Angular offset per parsec
	double m_radCore;        ///< Radius of the inner core
	double m_radGalaxy;      ///< Radius of the galaxy
	double m_radFarField;    ///< The radius after which all density waves must have circular shape
	double m_sigma;          ///< Distribution of stars
	double m_velAngle;       ///< Angular velocity of the density waves
	int m_numStars;          ///< Total number of stars
	int m_numDust;           ///< Number of Dust Particles
	int m_numH2;             ///< Number of H2 Regions
	double m_time;
	double m_timeStep;

	int m_numberByRad[100];  ///< Historgramm showing distribution of stars

	Vector3d m_pos;             ///< Center of the galaxy
	GallacticNode *m_pStars;          ///< Pointer to an array of star data
	GallacticNode *m_pDust;           ///< Pointer to an array of dusty areas
	GallacticNode *m_pH2;

	double *m_pStarCoords, *m_pStarColors;
	double *m_pDustCoords, *m_pDustColors;
	double *m_pH2Coords, *m_pH2Colors;

	Octree *octree;
};

#endif
