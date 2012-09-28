#include <iostream>

#include <StarSystem.h>

StarSystem::StarSystem() {
	std::cerr << "StarSystem initialized" << std::endl;
}

StarSystem::~StarSystem() {
	std::cerr << "StarSystem cleanup" << std::endl;
}

void StarSystem::step() {}
void StarSystem::draw() {}

