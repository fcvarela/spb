#include <GL/glfw.h>
#include <Common.h>
#include <Galaxy.h>
#include <Octree.h>

Octree::Octree(Octree *parent, uint64_t index, Vector3d &center, double size, unsigned short maxItemsPerNode) {
	// make sure we copy the center vector
	this->center = Vector3d(center);
	this->size = size;
	this->maxItemsPerNode = maxItemsPerNode;
	
	if (parent == NULL) {
		this->parent = NULL;
		this->index = 0;
	} else {
		this->parent = parent;
		this->index = this->parent->index * 10 + index;
	}

	for (uint8_t i=0; i<8; i++)
		children[i] = NULL;

	minx = center.x() - size/2.0;
	miny = center.y() - size/2.0;
	minz = center.z() - size/2.0;
	maxx = center.x() + size/2.0;
	maxy = center.y() + size/2.0;
	maxz = center.z() + size/2.0;

	boundingBox[0][0] = minx; boundingBox[0][1] = maxy; boundingBox[0][2] = minz;
	boundingBox[1][0] = minx; boundingBox[1][1] = maxy; boundingBox[1][2] = maxz;
	boundingBox[2][0] = maxx; boundingBox[2][1] = maxy; boundingBox[2][2] = minz;
	boundingBox[3][0] = maxx; boundingBox[3][1] = maxy; boundingBox[3][2] = maxz;
	boundingBox[4][0] = minx; boundingBox[4][1] = miny; boundingBox[4][2] = minz;
	boundingBox[5][0] = minx; boundingBox[5][1] = miny; boundingBox[5][2] = maxz;
	boundingBox[6][0] = maxx; boundingBox[6][1] = miny; boundingBox[6][2] = minz;
	boundingBox[7][0] = maxx; boundingBox[7][1] = miny; boundingBox[7][2] = maxz;

	starColors = NULL;
	starCoords = NULL;
}

Octree::~Octree() {
	items.clear();

	delete this->starCoords;
	delete this->starColors;

	for (uint8_t i=0; i<8; i++) {
		if (this->children[i] != NULL)
			delete this->children[i];
	}
}

bool Octree::containsPosition(Vector3d &position) {
	return (
		position.x() > center.x()-size/2.0 && position.x() < center.x()+size/2.0 &&
		position.y() > center.y()-size/2.0 && position.y() < center.y()+size/2.0 &&
		position.z() > center.z()-size/2.0 && position.z() < center.z()+size/2.0
	);
}

// build arrays to draw our stars
void Octree::synch() {
	if (children[0] != NULL) {
		for (uint8_t i=0; i<8; i++)
			children[i]->synch();
	}

	if (items.size() == 0)
		return;

	this->starCoords = new double[items.size() * 3];
	if (this->starCoords == NULL)
		exit(1);
	this->starColors = new double[items.size() * 3];
	if (this->starCoords == NULL)
		exit(1);

	// copy to array
	unsigned short i=0;
	for (std::list<GallacticNode *>::iterator s=items.begin(); s!=items.end(); ++s) {
		memcpy(&starCoords[i*3], (*s)->position, sizeof(double)*3);
		memcpy(&starColors[i*3], &((*s)->color), sizeof(double)*3);
		i++;
	}
}

Octree *Octree::nodeForPosition(Vector3d &position) {
	// recursively search for node where this needs to go
	if (children[0] != NULL) {
		for (uint8_t i=0; i<8; i++) {
			if (children[i]->containsPosition(position)) {
				return children[i]->nodeForPosition(position);
			}
		}
	}

	return this;
}

GallacticNode *Octree::nearestNode(Vector3d &position) {
	// get node for this position
	Octree *on = this->nodeForPosition(position);

	// get stars
	double mindistance = this->size * 10.0;
	GallacticNode *out = NULL;
	for (std::list<GallacticNode *>::iterator i=on->items.begin(); i!=on->items.end(); i++) {
		double distance = ((*i)->position - position).length();
		if (distance < mindistance) {
			mindistance = distance;
			out = (*i);
		}
	}

	return out;
}

void Octree::initChildren() {
	uint8_t i=0;
	for (int x = -1; x < 2; x+=2) {
		for (int y = -1; y < 2; y+=2) {
			for (int z = -1; z < 2; z+=2) {
				Vector3d c_center = this->center + Vector3d(x*size/4.0, y*size/4.0, z*size/4.0);
				children[i] = new Octree(this, i, c_center, size/2.0, this->maxItemsPerNode);
				i++;
			}
		}
	}

	// rebalance our data into our children
	for (std::list<GallacticNode *>::iterator i = items.begin(); i != items.end(); ++i) {
		for (uint8_t j=0; j<8; j++) {
			if (children[j]->containsPosition((*i)->position))
				children[j]->insertItem((*i));
		}
	}

	// clear our data
	this->items.clear();
}

// i really should template this...
void Octree::insertItem(GallacticNode *item) {
	// am i the one? make sure we can hold it
	if (this->items.size() < this->maxItemsPerNode && this->children[0]==NULL) {
		this->items.push_back(item);
		return;
	}

	// got here? we need to initialize our children
	if (this->children[0] == NULL)
		this->initChildren();

	Octree *node = this->nodeForPosition(item->position);
	node->insertItem(item);
}

void Octree::draw() {
	if (!boxInFrustum((double *)&boundingBox[0]))
		return;

	if (children[0] != NULL) {
		for (uint8_t i=0; i<8; i++)
			children[i]->draw();
		return;
	}

	if (this->starCoords == NULL)
		return;

	glVertexPointer(3, GL_DOUBLE, 0, this->starCoords);
	glColorPointer(3, GL_DOUBLE, 0, this->starColors);
	glDrawArrays(GL_POINTS, 0, items.size());
}

void Octree::drawColored() {
	if (!boxInFrustum((double *)&boundingBox[0]))
		return;

	if (children[0] != NULL) {
		for (uint8_t i=0; i<8; i++)
			children[i]->drawColored();
		return;
	}

	if (this->starCoords == NULL)
		return;

	GallacticNode *n;
	for (std::list<GallacticNode *>::iterator i = items.begin(); i != items.end(); ++i) {
		n = (*i);
		glColor3ub(n->colorid[0], n->colorid[1], n->colorid[2]);
		glVertex3d(n->position.x(), n->position.y(), n->position.z());
	}
}

void Octree::drawDebug() {
	glColor4f(0.0, 1.0, 0.0, 0.5);
	glBegin(GL_LINES);

	glVertex3d(minx, miny, minz);
	glVertex3d(minx, miny, maxz);
	
	glVertex3d(minx, maxy, minz);
	glVertex3d(minx, maxy, maxz);
	
	glVertex3d(maxx, miny, minz);
	glVertex3d(maxx, miny, maxz);
	
	glVertex3d(maxx, maxy, minz);
	glVertex3d(maxx, maxy, maxz);
	
	
	glVertex3d(minx, miny, minz);
	glVertex3d(maxx, miny, minz);
	
	glVertex3d(minx, maxy, minz);
	glVertex3d(maxx, maxy, minz);
	
	glVertex3d(minx, maxy, maxz);
	glVertex3d(maxx, maxy, maxz);
	
	glVertex3d(minx, miny, maxz);
	glVertex3d(maxx, miny, maxz);
	
	
	glVertex3d(minx, miny, minz);
	glVertex3d(minx, maxy, minz);
	
	glVertex3d(maxx, miny, minz);
	glVertex3d(maxx, maxy, minz);
	
	glVertex3d(minx, miny, maxz);
	glVertex3d(minx, maxy, maxz);
	
	glVertex3d(maxx, miny, maxz);
	glVertex3d(maxx, maxy, maxz);

	glEnd();
}