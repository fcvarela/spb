#include <iostream>
#include <StarSystemGenerator.h>
#include <StarSystem.h>
#include <Star.h>

StarSystemGenerator::StarSystemGenerator() {
	systemMapBuilder.SetSourceModule(generatorModule);
	systemMapBuilder.SetDestNoiseMap(systemMap);

	// generate up to 768*768 systems
	systemMapBuilder.SetDestSize(768, 768);
	systemMapBuilder.SetBounds(2.0, 6.0, 4.0, 8.1);

	systemMapBuilder.Build();
}

StarSystemGenerator::~StarSystemGenerator() {

}

void StarSystemGenerator::generateSystem(StarSystem *system) {
	// get this system's seed
	uint32_t y = system->seed / 768; 
	uint32_t x = system->seed % 768;
	float value = systemMap.GetValue(x, y);

	value = 1.0 + fabs(value);
	if (value > 1.0)
		value = value - 1.0;

	// star mass, radius, temperature
	// mass interval 0.013 to 300 solar masses
	double mass = 0.013 + value * 300.0;
	double radius = pow(mass, 0.8);

	// calculate temperature (m/ms)^1.5 = (t/ts)^4
	double fourth_temp = pow(mass, 1.5);
	double temp = pow(fourth_temp, 1/4.0) * 5800.0;

	system->star = new Star(system, radius * 6.955E8);

	// number of planets
	uint8_t planetcount = (int)(value*100.0)/8;

	// two categories. more than 6 and less than 6
	// less than 6? gas giants and telluric
	// more than 6? small rocks near star, middle 6 equals before, small rocks after last. telluric near, gas far

	// for each planet
	// mass, radius, semimajor axis, eccentricity, orbital period, orbital plane tilt, 

	// number of moons
}
