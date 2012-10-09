#include <sys/time.h>
#include <GameSceneManager.h>
#include <Common.h>
#include <Planet.h>
#include <Star.h>
#include <TerrainQuadtree.h>

Planet::Planet(const libconfig::Setting &planet, StarSystem *system, Node *parent) {
	this->system = system;
	this->parent = parent;
	
	if (parent == NULL)
		this->parent = system;

	// orbital stuff
	planet.lookupValue("semimajor_axis", this->semimajor_axis);
	planet.lookupValue("eccentricity", this->eccentricity);
	planet.lookupValue("orbital_period", this->orbital_period);
	planet.lookupValue("orbital_inclination", this->orbital_inclination);

	// physical dimension
	planet.lookupValue("radius", this->radius);
	this->atmosphere_radius = this->radius * 1.025;

	// name
	planet.lookupValue("name", this->label);

	// lod
	planet.lookupValue("maxlod", this->maxlod);

	GLUquadric *atmosphere = gluNewQuadric();
	gluQuadricNormals(atmosphere, GL_SMOOTH);

	_atmosphereDisplayList = glGenLists(1);
	_orbitDisplayList = glGenLists(1);

	// atmosphere
	glNewList(_atmosphereDisplayList, GL_COMPILE);
	gluSphere(atmosphere, this->atmosphere_radius, 50, 50);
	glEndList();
	
	// orbit
	glNewList(_orbitDisplayList, GL_COMPILE);
	glBegin(GL_LINE_LOOP);

	Quatd nrot = Quatd(Vector3d(1.0, 0.0, 0.0), this->orbital_inclination);
	for (int i=0; i<360; i++) {
		// compute mean anomaly
		double mean_anomaly = i * 0.01745;
	
		// compute eccentric anomaly (five iterations unrolled)
		double eccentric_anomaly;
		eccentric_anomaly = mean_anomaly - eccentricity * sin(mean_anomaly);
		eccentric_anomaly = mean_anomaly - eccentricity * sin(eccentric_anomaly);
		eccentric_anomaly = mean_anomaly - eccentricity * sin(eccentric_anomaly);
		eccentric_anomaly = mean_anomaly - eccentricity * sin(eccentric_anomaly);
		eccentric_anomaly = mean_anomaly - eccentricity * sin(eccentric_anomaly);
	
		// compute true anomaly
		double tan_half_ta = sqrt((1.0+eccentricity)/(1.0-eccentricity)) * tan(eccentric_anomaly/2.0);
		double true_anomaly = 2.0 * atan(tan_half_ta);

		// compute heliocentric distance
		double radius = semimajor_axis * (1.0 + eccentricity * cos(true_anomaly));
	
		// map to carthesian
		double c = cos(eccentric_anomaly);
		double s = sin(eccentric_anomaly);
	
		// update position
		Vector3d curpos = Vector3d(radius * sqrt(1.0-eccentricity*eccentricity)*s, 0.0, radius * c-eccentricity);
		nrot.rotate(curpos);
		glVertex3dv(curpos);
	}
	glEnd();
	glEndList();

	gluDeleteQuadric(atmosphere);

	this->atmosphereShader = new Shader("data/shaders/planet-atmosphere.glsl");
	this->surfaceShader = new Shader("data/shaders/planet-surface.glsl");
	this->time_scale = 0.0;

	// initialize our quadtrees
	Vector3d center, dx, dy;

	// top
	center = Vector3d(0.0, 0.5, 0.0);
	dx = Vector3d(1.0, 0.0, 0.0);
	dy = Vector3d(0.0, 0.0, -1.0);
	this->quadtrees[0] = new TerrainQuadtree(NULL, this, this->maxlod, 1, center, dx, dy);

	// left
	center = Vector3d(-0.5, 0.0, 0.0);
	dx = Vector3d(0.0, 0.0, 1.0);
	dy = Vector3d(0.0, 1.0, 0.0);
	this->quadtrees[1] = new TerrainQuadtree(NULL, this, this->maxlod, 2, center, dx, dy);

	// front
	center = Vector3d(0.0, 0.0, 0.5);
	dx = Vector3d(1.0, 0.0, 0.0);
	dy = Vector3d(0.0, 1.0, 0.0);
	this->quadtrees[2] = new TerrainQuadtree(NULL, this, this->maxlod, 3, center, dx, dy);

	// right
	center = Vector3d(0.5, 0.0, 0.0);
	dx = Vector3d(0.0, 0.0, -1.0);
	dy = Vector3d(0.0, 1.0, 0.0);
	this->quadtrees[3] = new TerrainQuadtree(NULL, this, this->maxlod, 4, center, dx, dy);

	// back
	center = Vector3d(0.0, 0.0, -0.5);
	dx = Vector3d(-1.0, 0.0, 0.0);
	dy = Vector3d(0.0, 1.0, 0.0);
	this->quadtrees[4] = new TerrainQuadtree(NULL, this, this->maxlod, 5, center, dx, dy);

	// bottom
	center = Vector3d(0.0, -0.5, 0.0);
	dx = Vector3d(1.0, 0.0, 0.0);
	dy = Vector3d(0.0, 0.0, 1.0);
	this->quadtrees[5] = new TerrainQuadtree(NULL, this, this->maxlod, 6, center, dx, dy);
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
	eccentric_anomaly = mean_anomaly - eccentricity * sin(mean_anomaly);
	eccentric_anomaly = mean_anomaly - eccentricity * sin(eccentric_anomaly);
	eccentric_anomaly = mean_anomaly - eccentricity * sin(eccentric_anomaly);
	eccentric_anomaly = mean_anomaly - eccentricity * sin(eccentric_anomaly);
	eccentric_anomaly = mean_anomaly - eccentricity * sin(eccentric_anomaly);

	// compute true anomaly
	double tan_half_ta = sqrt((1.0+eccentricity)/(1.0-eccentricity)) * tan(eccentric_anomaly/2.0);
	double true_anomaly = 2.0 * atan(tan_half_ta);

	// compute heliocentric distance
	double radius = semimajor_axis * (1.0 + eccentricity * cos(true_anomaly));

	// map to carthesian
	double c = cos(eccentric_anomaly);
	double s = sin(eccentric_anomaly);

	// update position
	this->position = Vector3d(radius * sqrt(1.0-eccentricity*eccentricity)*s, 0.0, radius * c-eccentricity);
	Quatd nrot = Quatd(Vector3d(1.0, 0.0, 0.0), this->orbital_inclination);
	nrot.rotate(this->position);

	this->position = this->parent->position + this->position;

	// step our moons
	for (std::list<Planet *>::iterator i = moons.begin(); i != moons.end(); ++i) {
		Planet *moon = *i;
		moon->step();
	}
}

Planet::~Planet() {}

void Planet::draw() {
	drawDebug();
	drawAtmosphere();
	drawSurface();

	// draw our moons
	for (std::list<Planet *>::iterator i = moons.begin(); i != moons.end(); ++i) {
		Planet *moon = *i;
		moon->draw();
	}
}

void Planet::drawDebug() {
	glDisable(GL_LIGHTING);
	// debug first
	glDepthMask(GL_FALSE);
	Node::draw();

	// draw our orbit
	/*
	glPushMatrix();
	glTranslated(parent->position.x(), parent->position.y(), parent->position.z());
	
	glColor4f(0.0, 1.0, 0.0, 0.4);
	glCallList(_orbitDisplayList);
	glPopMatrix();

	// debug orientation
	glPushMatrix();
	glColor3f(0.0, 0.0, 1.0);
	glBegin(GL_LINES);
	glVertex3d(position.x(), position.y()+radius*2.0, position.z());
	glVertex3d(position.x(), position.y()-radius*2.0, position.z());
	glEnd();
	glPopMatrix();
	*/
	glDepthMask(GL_TRUE);
	glEnable(GL_LIGHTING);
}

void Planet::drawAtmosphere() {
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

	GLuint shader = this->atmosphereShader->program;
	glUniform3f(glGetUniformLocation(shader, "v3CameraPos"),
		v3CameraPos.x(), v3CameraPos.y(), v3CameraPos.z());

	glUniform3f(glGetUniformLocation(shader, "v3LightPos"),
		v3LightPos.x(), v3LightPos.y(), v3LightPos.z());

	glUniform1f(glGetUniformLocation(shader, "fInnerRadius"), radius);

	// frustum
	glUniform1f(glGetUniformLocation(shader, "near"), __near__);
	glUniform1f(glGetUniformLocation(shader, "far"), __far__);

	glCallList(_atmosphereDisplayList);
	this->atmosphereShader->unbind();

	glFrontFace(GL_CCW);
	glDepthMask(GL_TRUE);
	glPopMatrix();
}

void Planet::drawSurface() {
	glPushMatrix();
	glTranslated(position.x(), position.y(), position.z());

	// draw surface
	this->surfaceShader->bind();
	GLuint shader = this->surfaceShader->program;

	// all positions relative to our center
	// set uniforms
	GameSceneManager *gsm = getGameSceneManager();
	Vector3d v3CameraPos = (gsm->camera->position - position/this->radius);
	Vector3d v3LightPos = system->star->position - position;
	v3LightPos.normalize();

	// tile textures
	glUniform1i(glGetUniformLocation(shader, "normalTexture"), 0);
	glUniform1i(glGetUniformLocation(shader, "colorTexture"), 1);
	glUniform1i(glGetUniformLocation(shader, "topoTexture"), 2);
        
    // tile parent textures
	glUniform1i(glGetUniformLocation(shader, "pnormalTexture"), 3);
	glUniform1i(glGetUniformLocation(shader, "pcolorTexture"), 4);
	glUniform1i(glGetUniformLocation(shader, "ptopoTexture"), 5);

    // near far
	glUniform1f(glGetUniformLocation(shader, "far"), __far__);

	// camera and light positions
	glUniform3f(glGetUniformLocation(shader, "v3CameraPos"),
		v3CameraPos.x(), v3CameraPos.y(), v3CameraPos.z());

	glUniform3f(glGetUniformLocation(shader, "v3LightPos"),
		v3LightPos.x(), v3LightPos.y(), v3LightPos.z());

	glUniform1f(glGetUniformLocation(shader, "fInnerRadius"), radius);

	// frustum
	glUniform1f(glGetUniformLocation(shader, "near"), __near__);
	glUniform1f(glGetUniformLocation(shader, "far"), __far__);

	for (uint8_t q=0; q<6; q++)
		this->quadtrees[q]->analyse(0.0);

	glPopMatrix();
	this->surfaceShader->unbind();
}
