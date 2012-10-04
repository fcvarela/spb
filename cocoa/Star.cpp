#include <Common.h>
#include <Star.h>
#include <StarSystem.h>

Star::Star(const libconfig::Setting &star, StarSystem *system) {
	this->system = system;
	star.lookupValue("name", this->label);
	star.lookupValue("radius", this->radius);
	this->position = system->position + Vector3d(star["position"][0], star["position"][1], star["position"][2]);

	GLfloat amb[4] = {.05, .05, .05, 1.0};
	GLfloat dif[4] = {.8, .8, .8, 1.0};
	GLfloat spe[4] = {.4, .4, .4, 1.0};

	memcpy(_ambientLightColor, amb, sizeof(GLfloat) * 4);
	memcpy(_diffuseLightColor, dif,  sizeof(GLfloat) * 4);
	memcpy(_specularLightColor, spe,  sizeof(GLfloat) * 4);

	// create the display list for this sphere
	GLUquadric *starq  = gluNewQuadric();
	gluQuadricNormals(starq, GL_SMOOTH);
	gluQuadricTexture(starq, GL_TRUE);
	_sphereDisplayList = glGenLists(1);
	glNewList(_sphereDisplayList, GL_COMPILE);
	gluSphere(starq, this->radius, 30, 30);
	glEndList();
	gluDeleteQuadric(starq);

	// instantiate our shader
	this->shader = new Shader("data/shaders/star.glsl");
}

Star::~Star() {}

void Star::draw() {
	this->shader->bind();
	glUniform1f(glGetUniformLocation(this->shader->program, "time"), __lasttime__);
	glPushMatrix();
	glTranslated(position.x(), position.y(), position.z());
	glCallList(_sphereDisplayList);
	glPopMatrix();
	this->shader->unbind();

	Node::draw();
}
