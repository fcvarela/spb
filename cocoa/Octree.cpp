#include <GL/glfw.h>
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
}

bool Octree::containsPosition(Vector3d &position) {
	return (
		position.x() > center.x()-size/2.0 && position.x() < center.x()+size/2.0 &&
		position.y() > center.y()-size/2.0 && position.y() < center.y()+size/2.0 &&
		position.z() > center.z()-size/2.0 && position.z() < center.z()+size/2.0
	);
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

unsigned long Octree::draw() {
	return 0;
	uint64_t totaldrawn = items.size();
	if (children[0] != NULL)
		for (uint8_t i=0; i<8; i++)
			totaldrawn += children[i]->draw();

	if (items.size() == 0)
		return 0;

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

	return totaldrawn;
}