#include <GL/glfw.h>
#include "Galaxy.h"
#include <Quaternion.h>
#include <SOIL.h>
#include <cstdlib>
#include <stdexcept>
#include <cmath>
#include <iostream>
#include "Constants.h"
#include "FastMath.h"
#include "CumulativeDistributionFunction.h"
#include "specrend.h"

GallacticNode::GallacticNode() {
	m_theta = 0.0;
	m_a = 0.0;
	m_b = 0.0;
	m_center = Vector3d(0,0,0);
}

const Vector3d& GallacticNode::CalcXY() {
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

	position = Vector3d(p.x() + (a * cosalpha * cosbeta - b * sinalpha * sinbeta),
		p.y() + (a * cosalpha * sinbeta + b * sinalpha * cosbeta), 0.0);

	double factor = 15.0;
	Quatd nrot = Quatd(Vector3d(1.0, 0.0, 0.0), 0.0);
	nrot = nrot * Quatd(Vector3d(1.0, 0.0, 0.0), m_inclinationx * factor);
	nrot = nrot * Quatd(Vector3d(0.0, 1.0, 0.0), m_inclinationy * factor);

	nrot.rotate(position);
	return position;
}

double Galaxy::my_random() {
	return ((double)rand()/(double)RAND_MAX);
}

Galaxy::~Galaxy() {
	delete [] m_pStars;
	delete [] m_pDust;
	delete [] m_pH2;
	delete [] m_pStarCoords;
	delete [] m_pStarColors;
	delete [] m_pDustCoords;
	delete [] m_pDustColors;
	delete [] m_pH2Coords;
	delete [] m_pH2Colors;
	FastMath::release();
}

Galaxy::Galaxy(double rad, double radCore, double deltaAng, double ex1, double ex2, double sigma, double velInner, double velOuter, int numStars) {
	m_elEx1 = ex1;
	m_elEx2 = ex2;
	m_velOrigin = 30;
	m_velInner = velInner;
	m_velOuter = velOuter;
	m_angleOffset = deltaAng;
	m_radCore = radCore;
	m_radGalaxy = rad;
	m_radFarField = m_radGalaxy * 2;
	m_sigma = sigma;
	m_velAngle = 0.000001;
	m_numStars = numStars;
	m_numDust = 11428;
	m_numH2 = 200;
	m_time = 0;
	m_timeStep = 0;
	m_pos.set(0.0, 0.0, 0.0);
	m_pStars = NULL;
	m_pDust = NULL;
	m_pH2 = NULL;
	
	FastMath::init();

	glGenTextures(1, &m_texStar);
	glBindTexture(GL_TEXTURE_2D, m_texStar);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_texStar = SOIL_load_OGL_texture("data/textures/particle.bmp", SOIL_LOAD_AUTO, m_texStar, SOIL_FLAG_MIPMAPS);
	std::cerr << m_texStar << std::endl;

	m_colNum = 200;
	m_t0 = 1000;
	m_t1 = 10000;
	m_dt = ((m_t1-m_t0)/m_colNum);

	double x, y, z;
	for (int i=0; i<m_colNum; ++i) {
		Color &col = m_col[i];
		colourSystem *cs = &SMPTEsystem;
		bbTemp = m_t0 + m_dt*i;
		spectrum_to_xyz(bb_spectrum, &x, &y, &z);
		xyz_to_rgb(cs, x, y, z, &col.r, &col.g, &col.b);
		norm_rgb(&col.r, &col.g, &col.b);
	}


	for (int i=0; i<100; ++i)
		m_numberByRad[i] = 0;

	InitStars(m_sigma);
}

double Galaxy::GetAngularOffset(double rad) const {
  return rad * m_angleOffset;
}

void Galaxy::InitStars(double sigma) {
	m_pDust = new GallacticNode[m_numDust];
	m_pStars = new GallacticNode[m_numStars];
	m_pH2 = new GallacticNode[m_numH2*2];

	m_pStarCoords = new double[m_numStars * 3];
	m_pStarColors = new double[m_numStars * 3];
	m_pDustCoords = new double[m_numDust * 3];
	m_pDustColors = new double[m_numDust * 3];
	m_pH2Coords = new double[m_numH2 * 3];
	m_pH2Colors = new double[m_numH2 * 3];

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
	cdf.SetupRealistic(1.0, 0.02, m_radGalaxy/3.0, m_radCore, 0, m_radFarField, 1000.0);
	for (int i=3; i<m_numStars; ++i) {
		double rad = cdf.ValFromProp(my_random());

		m_pStars[i].m_a = rad;
		m_pStars[i].m_b = rad * GetExcentricity(rad);
		m_pStars[i].m_angle = GetAngularOffset(rad);
		m_pStars[i].m_theta = 360.0 * my_random();
		m_pStars[i].m_inclinationx = my_random();
		m_pStars[i].m_inclinationy = my_random();
		m_pStars[i].m_velTheta = GetOrbitalVelocity(rad);
		m_pStars[i].m_center = Vector3d(0,0,0);
		m_pStars[i].m_temp = 6000.0 + (6000.0 * my_random()) - 3000.0;
		m_pStars[i].m_mag = 0.1 + 0.4 * my_random();

		int idx = std::min(1.0/dh * (m_pStars[i].m_a + m_pStars[i].m_b)/2.0, 99.0);
		m_numberByRad[idx]++;
	}

	// Initialise Dust
	double x,y,rad;
	for (int i=0; i<m_numDust; ++i) {
		x = 2*m_radGalaxy * my_random() - m_radGalaxy;
		y = 2*m_radGalaxy * my_random() - m_radGalaxy;
		rad = sqrt(x*x+y*y);

		m_pDust[i].m_a = rad;
		m_pDust[i].m_b = rad * GetExcentricity(rad);
		m_pDust[i].m_angle = GetAngularOffset(rad);
		m_pDust[i].m_theta = 360.0 * my_random();
		m_pDust[i].m_inclinationx = my_random();
		m_pDust[i].m_inclinationy = my_random();
		m_pDust[i].m_velTheta = GetOrbitalVelocity( (m_pDust[i].m_a + m_pDust[i].m_b)/2.0 );
		m_pDust[i].m_center = Vector3d(0,0,0);
		m_pDust[i].m_temp = 6000 + rad/4.0;

		m_pDust[i].m_mag = 0.015 + 0.01 * my_random();
		int idx = std::min(1.0/dh * (m_pDust[i].m_a + m_pDust[i].m_b)/2.0, 99.0);
		m_numberByRad[idx]++;
	}

	// Initialise Dust
	for (int i=0; i<m_numH2; ++i) {
		x = 2*(m_radGalaxy) * my_random() - (m_radGalaxy);
		y = 2*(m_radGalaxy) * my_random() - (m_radGalaxy);
		rad = sqrt(x*x+y*y);

		int k1 = 2*i;
		m_pH2[k1].m_a = rad;
		m_pH2[k1].m_b = rad * GetExcentricity(rad);
		m_pH2[k1].m_angle = GetAngularOffset(rad);
		m_pH2[k1].m_theta = 360.0 * my_random();
		m_pH2[k1].m_inclinationx = my_random();
		m_pH2[k1].m_inclinationy = my_random();
		m_pH2[k1].m_velTheta = GetOrbitalVelocity( (m_pH2[k1].m_a + m_pH2[k1].m_b)/2.0 );
		m_pH2[k1].m_center = Vector3d(0,0,0);
		m_pH2[k1].m_temp = 6000 + (6000 * my_random()) - 3000;
		m_pH2[k1].m_mag = 0.1 + 0.05 * my_random();
		int idx = std::min(1.0/dh * (m_pH2[k1].m_a + m_pH2[k1].m_b)/2.0, 99.0);
		m_numberByRad[idx]++;

		int k2 = 2*i+1;
		m_pH2[k2].m_a = rad + 1000;
		m_pH2[k2].m_b = rad * GetExcentricity(rad);
		m_pH2[k2].m_angle = m_pH2[k1].m_angle;
		m_pH2[k2].m_theta = m_pH2[k1].m_theta;
		m_pH2[k2].m_inclinationx = m_pH2[k1].m_inclinationx;
		m_pH2[k2].m_inclinationy = m_pH2[k1].m_inclinationy;
		m_pH2[k2].m_velTheta = m_pH2[k1].m_velTheta;
		m_pH2[k2].m_center = m_pH2[k1].m_center;
		m_pH2[k2].m_temp = m_pH2[k1].m_temp;
		m_pH2[k2].m_mag = m_pH2[k1].m_mag;
		idx = std::min(1.0/dh * (m_pH2[k2].m_a + m_pH2[k2].m_b)/2.0, 99.0);
		m_numberByRad[idx]++;
	}

	// set star colors
	for (int i=0; i<m_numStars; i++) {
		Color col = ColorFromTemperature(m_pStars[i].m_temp);
		col.r *= m_pStars[i].m_mag;
		col.g *= m_pStars[i].m_mag;
		col.b *= m_pStars[i].m_mag;
		memcpy(&m_pStarColors[i*3], &col, sizeof(double)*3);
	}

	for (int i=0; i<m_numDust; i++) {
		Color col = ColorFromTemperature(m_pDust[i].m_temp);
		col.r *= m_pDust[i].m_mag;
		col.g *= m_pDust[i].m_mag;
		col.b *= m_pDust[i].m_mag;
		memcpy(&m_pDustColors[i*3], &col, sizeof(double)*3);
	}

	for (int i=0; i<m_numH2; i++) {
		Color col = ColorFromTemperature(m_pH2[i].m_temp);
		col.r *= m_pH2[i].m_mag*2.0;
		col.g *= m_pH2[i].m_mag;
		col.b *= m_pH2[i].m_mag;
		memcpy(&m_pH2Colors[i*3], &col, sizeof(double)*3);
	}

	SingleTimeStep(100000);
}

double Galaxy::GetOrbitalVelocity(double rad) const {
	double vel_kms(0);  // velovity in kilometer per seconds

	// Calculate velovity in km per second
	if (rad<m_radCore) {
		double dv = (m_velInner-m_velOrigin) / m_radCore;
		vel_kms = m_velOrigin + rad * dv;
	} else if (rad>=m_radCore) {
		double dv = (m_velOuter-m_velInner) / (m_radGalaxy - m_radCore);
		vel_kms = m_velInner + dv * (rad-m_radCore);
	}

	// Calculate velocity in degree per year
	double u = 2 * M_PI * rad * Constant::PC_TO_KM;
	double time = u / (vel_kms * Constant::SEC_PER_YEAR);

	return 360.0 / time;
}

double Galaxy::GetExcentricity(double r) const {
	if (r<m_radCore) {
	// Core region of the galaxy. Innermost part is round
	// excentricity increasing linear to the border of the core.
		return 1 + (r / m_radCore) * (m_elEx1-1);
	} else if (r>m_radCore && r<=m_radGalaxy) {
		return m_elEx1 + (r-m_radCore) / (m_radGalaxy-m_radCore) * (m_elEx2-m_elEx1);
	} else if (r>m_radGalaxy && r <m_radFarField) {
		// excentricity is slowly reduced to 1.
		return m_elEx2 + (r - m_radGalaxy) / (m_radFarField - m_radGalaxy) * (1-m_elEx2);
	} else
		return 1;
}

void Galaxy::SingleTimeStep(double time) {
	m_timeStep = time;
	m_time += time;

	Vector3d posOld;
	for (int i=0; i<m_numStars; ++i) {
		m_pStars[i].m_theta += (m_pStars[i].m_velTheta * time);
		posOld = m_pStars[i].position;
		m_pStars[i].CalcXY();
		memcpy(&m_pStarCoords[i*3], m_pStars[i].position, sizeof(double)*3);

		Vector3d b = Vector3d(m_pStars[i].position.x() - posOld.x(), m_pStars[i].position.y() - posOld.y(), 0.0);
		m_pStars[i].velocity = b;
	}

	for (int i=0; i<m_numDust; ++i) {
		m_pDust[i].m_theta += (m_pDust[i].m_velTheta * time);
		posOld = m_pDust[i].position;
		m_pDust[i].CalcXY();
		memcpy(&m_pDustCoords[i*3], m_pDust[i].position, sizeof(double)*3);
	}

	for (int i=0; i<m_numH2*2; ++i) {
		m_pH2[i].m_theta += (m_pH2[i].m_velTheta * time);
		posOld = m_pDust[i].position;
		m_pH2[i].CalcXY();
		memcpy(&m_pH2Coords[i*3], m_pH2[i].position, sizeof(double)*3);
	}
}

void Galaxy::draw() {
	glDisable(GL_DEPTH_TEST);
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
	
	glPointSize(5.0);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_DOUBLE, 0, m_pStarCoords);
	glColorPointer(3, GL_DOUBLE, 0, m_pStarColors);
	glDrawArrays(GL_POINTS, 0, m_numStars);

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

	glPointSize(maxSize);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_DOUBLE, 0, m_pDustCoords);
	glColorPointer(3, GL_DOUBLE, 0, m_pDustColors);
	glDrawArrays(GL_POINTS, 0, 	m_numDust);

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

	/*
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_DOUBLE, 0, m_pH2Coords);
	glColorPointer(3, GL_DOUBLE, 0, m_pH2Colors);
	glPointSize(40);
	glDrawArrays(GL_POINTS, 0, m_numH2);
	glPointSize(15/6);
	glDrawArrays(GL_POINTS, 0, m_numH2);
	*/

	for (int i=0; i<m_numH2; ++i) {
		int k1 = 2*i;
		int k2 = 2*i+1;

		const Vector3d &p1 = m_pH2[k1].position;
		const Vector3d &p2 = m_pH2[k2].position;

		double dst = (p2-p1).length();
		double size = ((1000-dst) / 10) - 50;
		if (size<1)
			continue;

		glPointSize(2*size);
		glBegin(GL_POINTS);
		const Color &col = ColorFromTemperature(m_pH2[k1].m_temp);
		glColor3f(col.r * m_pH2[i].m_mag * 2,
			col.g * m_pH2[i].m_mag,
			col.b * m_pH2[i].m_mag);
		glVertex3f(p1.x(), p1.y(), p1.z());
		glEnd();

		glPointSize(size/6);
		glBegin(GL_POINTS);
		glColor3f(1,1,1);
		glVertex3f(p1.x(), p1.y(), p1.z());
		glEnd();
	}
	
	glDisable(GL_POINT_SPRITE);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}

Color Galaxy::ColorFromTemperature(double temp) const {
	int idx = (temp - m_t0) / (m_t1-m_t0) * m_colNum;
	idx = std::min(m_colNum-1, idx);
	idx = std::max(0, idx);
	return m_col[idx];
}
