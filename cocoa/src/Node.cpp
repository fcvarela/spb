#include <Common.h>
#include <Node.h>

uint32_t Node::g_id;

Node::Node() {
	rotation = Quatd(1.0, 0.0, 0.0, 0.0);
	position = Vector3d(0.0, 0.0, 0.0);

	// increment
	g_id++;

	// copy
	uint32_t bigendian = htonl(g_id);
	uint8_t *p = (uint8_t *)&bigendian;

	colorid[0] = p[1];
	colorid[1] = p[2];
	colorid[2] = p[3];

	label = "";
}

void Node::step() {}

void Node::draw() {
	// debug label
	glColor3f(1.0, 1.0, 1.0);
	glRasterPos3f(position.x(), position.y(), position.z());
	__font__->Render(this->label.c_str());
}

void Node::coloridToLabel() {
	uint32_t id = colorid[2] + colorid[1] * 256 + colorid[0] * 65536;
	while (id > 10) {
		uint8_t digit = 65 + id%10;
		label.push_back(digit);
		id /= 10;
	}
	std::cerr << label << std::endl;
}