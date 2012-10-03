#include <sys/time.h>
#include <Common.h>
#include <Planet.h>

Planet::Planet(const libconfig::Setting &planet, Node *system) {
	this->system = system;

	// orbital stuff
	planet.lookupValue("semimajor_axis", this->semimajor_axis);
	planet.lookupValue("eccentricity", this->eccentricity);
	planet.lookupValue("orbital_period", this->orbital_period);

	// physical dimension
	planet.lookupValue("radius", this->radius);
	this->atmosphere_radius = this->radius * 1.025;

	// name
	planet.lookupValue("name", this->label);

	GLUquadric *surface  = gluNewQuadric();
	gluQuadricNormals(surface, GL_SMOOTH);
	_sphereDisplayList = glGenLists(1);
	glNewList(_sphereDisplayList, GL_COMPILE);
	gluSphere(surface, this->radius, 50, 50);
	glEndList();
	gluDeleteQuadric(surface);

	time_scale = 0.0;
}

void Planet::step() {
	// polar coordinates: angle, radius
	struct timeval tv;
	gettimeofday(&tv, NULL);

	double localtime = tv.tv_sec + (tv.tv_usec/1000000.0);

	// compute mean anomaly
	double mean_anomaly = (2.0*M_PI*localtime) / (this->orbital_period*24.0*60.0*60.0);

	// compute eccentric anomaly (five iterations unrolled)
	double eccentric_anomaly;
	eccentric_anomaly = mean_anomaly + eccentricity * sin(mean_anomaly);
	eccentric_anomaly = mean_anomaly + eccentricity * sin(eccentric_anomaly);
	eccentric_anomaly = mean_anomaly + eccentricity * sin(eccentric_anomaly);
	eccentric_anomaly = mean_anomaly + eccentricity * sin(eccentric_anomaly);
	eccentric_anomaly = mean_anomaly + eccentricity * sin(eccentric_anomaly);

	// compute heliocentric distance
	double radius = semimajor_axis * (1.0 - eccentricity * cos(eccentric_anomaly));

	// map to carthesian
	double c = cos(eccentric_anomaly);
	double s = sin(eccentric_anomaly);

	this->position = Vector3d(radius * sqrt(1.0-eccentricity*eccentricity)*s, 0.0, radius * c-eccentricity);
	this->position = this->system->position + this->position;
}

Planet::~Planet() {}

void Planet::draw() {
	glPushMatrix();
	glTranslated(position.x(), position.y(), position.z());
	glColor4f(1.0, 0.0, 0.0, 1.0);
	glCallList(_sphereDisplayList);
	glPopMatrix();

	glDisable(GL_DEPTH_TEST);
	glColor3f(1.0, 0.0, 0.0);
	glRasterPos3f(position.x(), position.y(), position.z());
	__font__->Render(this->label.c_str());
	glEnable(GL_DEPTH_TEST);
}
