#include <Common.h>
#include <Star.h>
#include <StarSystem.h>

Star::Star(StarSystem *system, double radius) {
	this->radius = radius;
	this->position = system->position;

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
	glUniform1f(glGetUniformLocation(this->shader->program, "near"), __near__);
	glUniform1f(glGetUniformLocation(this->shader->program, "far"), __far__);
	glPushMatrix();
	glTranslated(position.x(), position.y(), position.z());
	glCallList(_sphereDisplayList);
	glPopMatrix();
	this->shader->unbind();

	Node::draw();
}
