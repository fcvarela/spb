#include <sys/time.h>
#include <GameSceneManager.h>
#include <Common.h>
#include <Planet.h>
#include <Star.h>

Planet::Planet(const libconfig::Setting &planet, StarSystem *system, Node *parent) {
	this->system = system;
	this->parent = parent;
	
	if (parent == NULL)
		this->parent = system;

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
	GLUquadric *atmosphere = gluNewQuadric();

	gluQuadricNormals(surface, GL_SMOOTH);
	gluQuadricNormals(atmosphere, GL_SMOOTH);

	_surfaceDisplayList = glGenLists(1);
	_atmosphereDisplayList = glGenLists(1);

	glNewList(_surfaceDisplayList, GL_COMPILE);
	gluSphere(surface, this->radius, 50, 50);
	glEndList();

	glNewList(_atmosphereDisplayList, GL_COMPILE);
	gluSphere(atmosphere, this->atmosphere_radius, 50, 50);
	glEndList();
	
	gluDeleteQuadric(surface);
	gluDeleteQuadric(atmosphere);

	atmosphereShader = new Shader("data/shaders/planet-atmosphere.glsl");
	surfaceShader = new Shader("planet-surface.glsl");

	time_scale = 0.0;
}

void Planet::step() {
	// polar coordinates: angle, radius (Kepler method)
	struct timeval tv;
	gettimeofday(&tv, NULL);

	double localtime = tv.tv_sec + (tv.tv_usec/1000000.0) + time_scale;
	time_scale += 0.0;

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

	// update position
	this->position = Vector3d(radius * sqrt(1.0-eccentricity*eccentricity)*s, 0.0, radius * c-eccentricity);
	this->position = this->parent->position + this->position;

	// step our moons
	for (std::list<Planet *>::iterator i = moons.begin(); i != moons.end(); ++i) {
		Planet *moon = *i;
		moon->step();
	}
}

Planet::~Planet() {}

void Planet::draw() {
	glPushMatrix();

	glTranslated(position.x(), position.y(), position.z());
	
	// draw atmosphere
	glDepthMask(GL_FALSE);
	glFrontFace(GL_CW);
	this->atmosphereShader->bind();

	// set uniforms
	GameSceneManager *gsm = getGameSceneManager();

	// all positions relative to our center
	Vector3d v3CameraPos = gsm->camera->position - position;
	Vector3d v3LightPos = system->star->position - position;
	v3LightPos.normalize();

	glUniform3f(
		glGetUniformLocation(this->atmosphereShader->program, "v3CameraPos"),
		v3CameraPos.x(), v3CameraPos.y(), v3CameraPos.z());

	glUniform3f(
		glGetUniformLocation(this->atmosphereShader->program, "v3LightPos"),
		v3LightPos.x(), v3LightPos.y(), v3LightPos.z());

	glUniform3f(
		glGetUniformLocation(this->atmosphereShader->program, "v3PlanetCenter"),
		position.x(), position.y(), position.z());

	glUniform1f(glGetUniformLocation(this->atmosphereShader->program, "fInnerRadius"), radius);

	glCallList(_atmosphereDisplayList);
	this->atmosphereShader->unbind();

	glFrontFace(GL_CCW);
	glDepthMask(GL_TRUE);

	//this->surfaceShader->bind();
	glColor3f(1.0, 0.0, 0.0);
	glCallList(_surfaceDisplayList);
	//this->surfaceShader->unbind();
	glPopMatrix();

	Node::draw();

	// draw our moons
	for (std::list<Planet *>::iterator i = moons.begin(); i != moons.end(); ++i) {
		Planet *moon = *i;
		moon->draw();
	}
}
