#include <GL/glfw.h>
#include "Galaxy.h"
#include <SOIL.h>
#include <cstdlib>
#include <stdexcept>
#include <cmath>
#include <iostream>
#include "Constants.h"
#include "FastMath.h"
#include "CumulativeDistributionFunction.h"
#include "specrend.h"


//------------------------------------------------------------------------
double rnd_spread(double v, double o)
{
	return (v + (2*o * (double)rand()/RAND_MAX - o));
}

//------------------------------------------------------------------------
GallacticStar::GallacticStar()
:m_theta(0)
,m_a(0)
,m_b(0)
,m_center(0,0,0)
{}

//-----------------------------------------------------------------------
const Vector3d& GallacticStar::CalcXY()
{
	double &a = m_a,
	&b = m_b,
	&theta = m_theta;
	const Vector3d &p = m_center;

	double beta  = -m_angle,
	alpha = theta * Constant::DEG_TO_RAD;

	// temporaries to save cpu time
	double cosalpha = cos(alpha),
	sinalpha = sin(alpha),
	cosbeta  = cos(beta),
	sinbeta  = sin(beta);

	m_pos = Vector3d(p.x() + (a * cosalpha * cosbeta - b * sinalpha * sinbeta),
		p.y() + (a * cosalpha * sinbeta + b * sinalpha * cosbeta), 0.0);
	return m_pos;
}

//------------------------------------------------------------------------
Galaxy::Galaxy(double rad,
	double radCore,
	double deltaAng,
	double ex1,
	double ex2,
	double velInner,
	double velOuter,
	int numStars)
:m_elEx1(ex1)
,m_elEx2(ex2)
,m_velOrigin(30)
,m_velInner(velInner)
,m_velOuter(velOuter)
,m_angleOffset(deltaAng)
,m_radCore(radCore)
,m_radGalaxy(rad)
,m_sigma(0.45)
,m_velAngle(0.000001)
,m_numStars(numStars)
,m_numDust(numStars/3.5)
,m_numH2(200)
,m_time(0)
,m_timeStep(0)
,m_pos(0, 0, 0)
,m_pStars(NULL)
,m_pDust(NULL)
,m_pH2(NULL)
{
	FastMath::init();
	glGenTextures(1, &m_texStar);
	glBindTexture(GL_TEXTURE_2D, m_texStar);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  	m_texStar = SOIL_load_OGL_texture("data/textures/particle.bmp", SOIL_LOAD_AUTO, m_texStar, SOIL_FLAG_MIPMAPS);
  	std::cerr << m_texStar << std::endl;
}

//------------------------------------------------------------------------
Galaxy::~Galaxy()
{
	delete [] m_pStars;
	delete [] m_pDust;
	delete [] m_pH2;
	FastMath::release();
}

//------------------------------------------------------------------------
void Galaxy::Reset()
{
	Reset(m_radGalaxy,
		m_radCore,
		m_angleOffset,
		m_elEx1,
		m_elEx2,
		m_sigma,
		m_velInner,
		m_velOuter,
		m_numStars);
}

//------------------------------------------------------------------------
void Galaxy::Reset(double rad,
	double radCore,
	double deltaAng,
	double ex1,
	double ex2,
	double sigma,
	double velInner,
	double velOuter,
	int numStars)
{
	m_elEx1 = ex1;
	m_elEx2 = ex2;
	m_velInner = velInner;
	m_velOuter = velOuter;
	m_elEx2 = ex2;
	m_angleOffset = deltaAng;
	m_radCore = radCore;
	m_radGalaxy = rad;
	m_radFarField = m_radGalaxy * 2;  // there is no science behind this threshold it just should look nice
	m_sigma = sigma;
	m_numStars = numStars;
	m_numDust = numStars/2;
	m_time = 0;

	for (int i=0; i<100; ++i)
		m_numberByRad[i] = 0;

	InitStars(m_sigma);
}

//------------------------------------------------------------------------
void Galaxy::InitStars(double sigma)
{
	delete [] m_pDust;
	m_pDust = new GallacticStar[m_numDust];

	delete [] m_pStars;
	m_pStars = new GallacticStar[m_numStars];

	delete [] m_pH2;
	m_pH2 = new GallacticStar[m_numH2*2];

	// The first three stars can be used for aligning the
	// camera with the galaxy rotation.

	// First star ist the black hole at the centre
	m_pStars[0].m_a = 0.0;
	m_pStars[0].m_b = 0.0;
	m_pStars[0].m_angle = 0.0;
	m_pStars[0].m_theta = 0.0;
	m_pStars[0].m_velTheta = 0.0;
	m_pStars[0].m_center = Vector3d(0.0,0.0,0.0);
	m_pStars[0].m_velTheta = GetOrbitalVelocity( (m_pStars[0].m_a + m_pStars[0].m_b)/2.0 );
	m_pStars[0].m_temp = 6000.0;

	// second star is at the edge of the core area
	m_pStars[1].m_a = m_radCore;
	m_pStars[1].m_b = m_radCore * GetExcentricity(m_radCore);
	m_pStars[1].m_angle = GetAngularOffset(m_radCore);
	m_pStars[1].m_theta = 0.0;
	m_pStars[1].m_center = Vector3d(0.0,0.0,0.0);
	m_pStars[1].m_velTheta = GetOrbitalVelocity( (m_pStars[1].m_a + m_pStars[1].m_b)/2.0 );
	m_pStars[1].m_temp = 6000.0;

	// third star is at the edge of the disk
	m_pStars[2].m_a = m_radGalaxy;
	m_pStars[2].m_b = m_radGalaxy * GetExcentricity(m_radGalaxy);
	m_pStars[2].m_angle = GetAngularOffset(m_radGalaxy);
	m_pStars[2].m_theta = 0.0;
	m_pStars[2].m_center = Vector3d(0.0,0.0,0.0);
	m_pStars[2].m_velTheta = GetOrbitalVelocity( (m_pStars[2].m_a + m_pStars[2].m_b)/2.0 );
	m_pStars[2].m_temp = 6000.0;

	// cell width of the histogramm
	double dh = (double)m_radFarField/100.0;

	// Initialize the stars
	CumulativeDistributionFunction cdf;
	cdf.SetupRealistic(1.0,             // Maximalintensität
					 0.02,            // k (bulge)
					 m_radGalaxy/3.0, // disc skalenlänge
					 m_radCore,       // bulge radius
					 0,               // start der intensitätskurve
					 m_radFarField,   // ende der intensitätskurve
					 1000.0);           // Anzahl der stützstellen
	for (int i=3; i<m_numStars; ++i) {
		double rad = cdf.ValFromProp((double)rand()/(double)RAND_MAX);

		m_pStars[i].m_a = rad;
		m_pStars[i].m_b = rad * GetExcentricity(rad);
		m_pStars[i].m_angle = GetAngularOffset(rad);
		m_pStars[i].m_theta = 360.0 * ((double)rand() / RAND_MAX);
		m_pStars[i].m_inclination = 15.0 * ((double)rand() / RAND_MAX);
		m_pStars[i].m_velTheta = GetOrbitalVelocity(rad);
		m_pStars[i].m_center = Vector3d(0,0,0);
		m_pStars[i].m_temp = 6000.0 + (6000.0 * ((double)rand() / RAND_MAX)) - 3000.0;
		m_pStars[i].m_mag = 0.1 + 0.4 * (double)rand()/(double)RAND_MAX;

		int idx = std::min(1.0/dh * (m_pStars[i].m_a + m_pStars[i].m_b)/2.0, 99.0);
		m_numberByRad[idx]++;
	}

	// Initialise Dust
	double x,y,rad;
	for (int i=0; i<m_numDust; ++i)
	{
		x = 2*m_radGalaxy * ((double)rand() / RAND_MAX) - m_radGalaxy;
		y = 2*m_radGalaxy * ((double)rand() / RAND_MAX) - m_radGalaxy;
		rad = sqrt(x*x+y*y);

		m_pDust[i].m_a = rad;
		m_pDust[i].m_b = rad * GetExcentricity(rad);
		m_pDust[i].m_angle = GetAngularOffset(rad);
		m_pDust[i].m_theta = 360.0 * ((double)rand() / RAND_MAX);
		m_pDust[i].m_velTheta = GetOrbitalVelocity( (m_pDust[i].m_a + m_pDust[i].m_b)/2.0 );
		m_pDust[i].m_center = Vector3d(0,0,0);
		m_pDust[i].m_temp = 6000 + rad/4.0;

		m_pDust[i].m_mag = 0.015 + 0.01 * (double)rand()/(double)RAND_MAX;
		int idx = std::min(1.0/dh * (m_pDust[i].m_a + m_pDust[i].m_b)/2.0, 99.0);
		m_numberByRad[idx]++;
	}

	// Initialise Dust
	for (int i=0; i<m_numH2; ++i)
	{
		x = 2*(m_radGalaxy) * ((double)rand() / RAND_MAX) - (m_radGalaxy);
		y = 2*(m_radGalaxy) * ((double)rand() / RAND_MAX) - (m_radGalaxy);
		rad = sqrt(x*x+y*y);

		int k1 = 2*i;
		m_pH2[k1].m_a = rad;
		m_pH2[k1].m_b = rad * GetExcentricity(rad);
		m_pH2[k1].m_angle = GetAngularOffset(rad);
		m_pH2[k1].m_theta = 360.0 * ((double)rand() / RAND_MAX);
		m_pH2[k1].m_velTheta = GetOrbitalVelocity( (m_pH2[k1].m_a + m_pH2[k1].m_b)/2.0 );
		m_pH2[k1].m_center = Vector3d(0,0,0);
		m_pH2[k1].m_temp = 6000 + (6000 * ((double)rand() / RAND_MAX)) - 3000;
		m_pH2[k1].m_mag = 0.1 + 0.05 * (double)rand()/(double)RAND_MAX;
		int idx = std::min(1.0/dh * (m_pH2[k1].m_a + m_pH2[k1].m_b)/2.0, 99.0);
		m_numberByRad[idx]++;

		int k2 = 2*i+1;
		m_pH2[k2].m_a = rad + 1000;
		m_pH2[k2].m_b = rad * GetExcentricity(rad);
	m_pH2[k2].m_angle = m_pH2[k1].m_angle; //GetAngularOffset(rad);
	m_pH2[k2].m_theta = m_pH2[k1].m_theta;
	m_pH2[k2].m_velTheta = m_pH2[k1].m_velTheta;
	m_pH2[k2].m_center = m_pH2[k1].m_center;
	m_pH2[k2].m_temp = m_pH2[k1].m_temp;
	m_pH2[k2].m_mag = m_pH2[k1].m_mag;
	idx = std::min(1.0/dh * (m_pH2[k2].m_a + m_pH2[k2].m_b)/2.0, 99.0);
	m_numberByRad[idx]++;
}
}

//------------------------------------------------------------------------
double Galaxy::GetSigma() const
{
	return m_sigma;
}

//------------------------------------------------------------------------
void Galaxy::SetSigma(double s)
{
	m_sigma = s;
	Reset();
}

//------------------------------------------------------------------------
GallacticStar* Galaxy::GetStars() const
{
	return m_pStars;
}

//------------------------------------------------------------------------
GallacticStar* Galaxy::GetDust() const
{
	return m_pDust;
}

//------------------------------------------------------------------------
GallacticStar* Galaxy::GetH2() const
{
	return m_pH2;
}

//------------------------------------------------------------------------
double Galaxy::GetRad() const
{
	return m_radGalaxy;
}

//------------------------------------------------------------------------
double Galaxy::GetCoreRad() const
{
	return m_radCore;
}

//------------------------------------------------------------------------
double Galaxy::GetFarFieldRad() const
{
	return m_radFarField;
}

//------------------------------------------------------------------------
void Galaxy::SetAngularOffset(double offset)
{
	m_angleOffset = offset;
	Reset();
}

//------------------------------------------------------------------------
/** \brief Returns the orbital velocity in degrees per year.
	\param rad Radius in parsec
*/
	double Galaxy::GetOrbitalVelocity(double rad) const
	{
	double vel_kms(0);  // velovity in kilometer per seconds

	// Calculate velovity in km per second
	if (rad<m_radCore)
	{
		double dv = (m_velInner-m_velOrigin) / m_radCore;
		vel_kms = m_velOrigin + rad * dv;
	}
	else if (rad>=m_radCore)
	{
		double dv = (m_velOuter-m_velInner) / (m_radGalaxy - m_radCore);
		vel_kms = m_velInner + dv * (rad-m_radCore);
	}

	// Calculate velocity in degree per year
	double u = 2 * M_PI * rad * Constant::PC_TO_KM;        // Umfang in km
	double time = u / (vel_kms * Constant::SEC_PER_YEAR);  // Umlaufzeit in Jahren

	return 360.0 / time;                                   // Grad pro Jahr

//  return 0.000005;
}

//------------------------------------------------------------------------
double Galaxy::GetExcentricity(double r) const
{
	if (r<m_radCore)
	{
	// Core region of the galaxy. Innermost part is round
	// excentricity increasing linear to the border of the core.
		return 1 + (r / m_radCore) * (m_elEx1-1);
	}
	else if (r>m_radCore && r<=m_radGalaxy)
	{
		return m_elEx1 + (r-m_radCore) / (m_radGalaxy-m_radCore) * (m_elEx2-m_elEx1);
	}
	else if (r>m_radGalaxy && r <m_radFarField)
	{
	// excentricity is slowly reduced to 1.
		return m_elEx2 + (r - m_radGalaxy) / (m_radFarField - m_radGalaxy) * (1-m_elEx2);
	}
	else
		return 1;
}

//------------------------------------------------------------------------
double Galaxy::GetAngularOffset(double rad) const
{
	return rad * m_angleOffset;
}

//------------------------------------------------------------------------
double Galaxy::GetAngularOffset() const
{
	return m_angleOffset;
}

//------------------------------------------------------------------------
double Galaxy::GetExInner() const
{
	return m_elEx1;
}

//-----------------------------------------------------------------------
double Galaxy::GetExOuter() const
{
	return m_elEx2;
}

//-----------------------------------------------------------------------
void Galaxy::SetRad(double rad)
{
	m_radGalaxy = rad;
	Reset();
}

//-----------------------------------------------------------------------
void Galaxy::SetCoreRad(double rad)
{
	m_radCore = rad;
	Reset();
}

//-----------------------------------------------------------------------
void Galaxy::SetExInner(double ex)
{
	m_elEx1 = ex;
	Reset();
}

//-----------------------------------------------------------------------
void Galaxy::SetExOuter(double ex)
{
	m_elEx2 = ex;
	Reset();
}

//-----------------------------------------------------------------------
double Galaxy::GetTimeStep() const
{
	return m_timeStep;
}

//-----------------------------------------------------------------------
double Galaxy::GetTime() const
{
	return m_time;
}

//-----------------------------------------------------------------------
void Galaxy::SingleTimeStep(double time)
{
	m_timeStep = time;
	m_time += time;

	Vector3d posOld;
	for (int i=0; i<m_numStars; ++i)
	{
		m_pStars[i].m_theta += (m_pStars[i].m_velTheta * time);
		posOld = m_pStars[i].m_pos;
		m_pStars[i].CalcXY();

		Vector3d b = Vector3d(m_pStars[i].m_pos.x() - posOld.x(),
			m_pStars[i].m_pos.y() - posOld.y(), 0.0);
		m_pStars[i].m_vel = b;
	}

	for (int i=0; i<m_numDust; ++i)
	{
		m_pDust[i].m_theta += (m_pDust[i].m_velTheta * time);
		posOld = m_pDust[i].m_pos;
		m_pDust[i].CalcXY();
	}

	for (int i=0; i<m_numH2*2; ++i)
	{
		m_pH2[i].m_theta += (m_pH2[i].m_velTheta * time);
		posOld = m_pDust[i].m_pos;
		m_pH2[i].CalcXY();
	}

}

int Galaxy::GetNumH2() const
{
	return m_numH2;
}


//-----------------------------------------------------------------------
int Galaxy::GetNumStars() const
{
	return m_numStars;
}

//-----------------------------------------------------------------------
int Galaxy::GetNumDust() const
{
	return m_numDust;
}

//-----------------------------------------------------------------------
const Vector3d& Galaxy::GetStarPos(int idx)
{
	if (idx>=m_numStars)
		throw std::runtime_error("index out of bounds.");

	return m_pStars[idx].m_pos; //GetPos();
}

void Galaxy::draw() {
	SingleTimeStep(100000);
	glDisable(GL_DEPTH_TEST);
	glColor3f(1,1,1);

	drawDust();
	drawH2();
	drawStars();
	glEnable(GL_DEPTH_TEST);
}

void Galaxy::drawStars() {
	glBindTexture(GL_TEXTURE_2D, m_texStar);

	float maxSize = 0.0f;
	glGetFloatv(GL_POINT_SIZE_MAX, &maxSize );
	glPointParameterf(GL_POINT_SIZE_MAX, maxSize);
	glPointParameterf(GL_POINT_SIZE_MIN, 1.0f);
	glTexEnvf(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);

	glEnable(GL_POINT_SPRITE);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	int num = GetNumStars();
	GallacticStar *pStars = GetStars();

	glPointSize(4.0);
	glBegin(GL_POINTS);
	glColor3f(1.0, 1.0, 1.0);
	for (int i=1; i<num; ++i) {
		//const Color &col = ColorFromTemperature(pStars[i].m_temp);
		/*glColor3f(col.r * pStars[i].m_mag,
				col.g * pStars[i].m_mag,
				col.b * pStars[i].m_mag);*/
		glVertex3f(pStars[i].m_pos.x(), pStars[i].m_pos.y(), 0.0f);

	}
	glEnd();
	glDisable(GL_POINT_SPRITE);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
}

void Galaxy::drawDust() {
	glBindTexture(GL_TEXTURE_2D, m_texStar);
	float maxSize = 0.0f;
	glGetFloatv( GL_POINT_SIZE_MAX, &maxSize );
	glPointParameterf(GL_POINT_SIZE_MAX, maxSize);
	glPointParameterf(GL_POINT_SIZE_MIN, 1.0f);
	glTexEnvf(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
	glEnable(GL_POINT_SPRITE);
	glEnable(GL_TEXTURE_2D);       // point sprite texture support
	glEnable(GL_BLEND);            // soft blending of point sprites
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	GallacticStar *pDust = GetDust();
	int num = GetNumDust();

	glPointSize(1.0);//maxSize);
	glBegin(GL_POINTS);

	for (int i=0; i<num; ++i) {
		const Vector3d &pos = pDust[i].m_pos;
		/*const Color &col = ColorFromTemperature(pDust[i].m_temp);
		glColor3f(col.r * pDust[i].m_mag,
			col.g * pDust[i].m_mag,
			col.b * pDust[i].m_mag);*/
		glVertex3f(pos.x(), pos.y(), 0.0f);

	}
	glEnd();
	glDisable(GL_POINT_SPRITE);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}

void Galaxy::drawH2() {
	glBindTexture(GL_TEXTURE_2D, m_texStar);
	float maxSize = 0.0f;
	glGetFloatv( GL_POINT_SIZE_MAX, &maxSize );
	glPointParameterf(GL_POINT_SIZE_MAX, maxSize);
	glPointParameterf(GL_POINT_SIZE_MIN, 1.0f);
	glTexEnvf(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);

	glEnable(GL_POINT_SPRITE);
	glEnable(GL_TEXTURE_2D);       // point sprite texture support
	glEnable(GL_BLEND);            // soft blending of point sprites
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	GallacticStar *pH2 = GetH2();
	int num = GetNumH2();

	for (int i=0; i<num; ++i) {
		int k1 = 2*i;
		int k2 = 2*i+1;

		const Vector3d &p1 = pH2[k1].m_pos;
		const Vector3d &p2 = pH2[k2].m_pos;

		double dst = (p2-p1).length();
		double size = ((1000-dst) / 10) - 50;
		if (size<1)
			continue;

		glPointSize(1.0);//2*size);
		glBegin(GL_POINTS);
		/*const Color &col = ColorFromTemperature(pH2[k1].m_temp);
		glColor3f(col.r * pH2[i].m_mag * 2,
			col.g * pH2[i].m_mag,
			col.b * pH2[i].m_mag);*/
		glVertex3f(p1.x(), p1.y(), 0.0f);
		glEnd();

		glPointSize(size/6);
		glBegin(GL_POINTS);
		glColor3f(1,1,1);
		glVertex3f(p1.x(), p1.y(), 0.0f);
		glEnd();
	}
	glDisable(GL_POINT_SPRITE);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}


void Galaxy::DrawEllipsis(double a, double b, double angle) {
  const int steps = 100;
  const double x = 0;
  const double y = 0;

  // Angle is given by Degree Value
  double beta = -angle * M_PI / 180; //(Math.PI/180) converts Degree Value into Radians
  double sinbeta = sin(beta);
  double cosbeta = cos(beta);

  glBegin(GL_LINE_STRIP);

  for (int i=0; i<361; i += 360 / steps)
  {
    double alpha = i * (M_PI / 180) ;
    double sinalpha = sin(alpha);
    double cosalpha = cos(alpha);

    double X = x + (a * cosalpha * cosbeta - b * sinalpha * sinbeta);
    double Y = y + (a * cosalpha * sinbeta + b * sinalpha * cosbeta);

    glVertex3f(X, Y, 0);
   }
   glEnd();
}