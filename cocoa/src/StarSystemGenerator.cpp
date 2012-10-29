#include <iostream>
#include <StarSystemGenerator.h>

StarSystemGenerator::StarSystemGenerator() {
	systemMapBuilder.SetSourceModule(generatorModule);
	systemMapBuilder.SetDestNoiseMap(systemMap);

	// generate up to 768*768 systems
	systemMapBuilder.SetDestSize(768, 768);
	systemMapBuilder.SetBounds(2.0, 6.0, 4.0, 8.1);

	systemMapBuilder.Build();
	
	for (uint16_t y=0; y<768; y++) {
		for (uint16_t x=0; x<768; x++) {
			generateSystem(y*768 + x);
		}
	}
}

StarSystemGenerator::~StarSystemGenerator() {

}

void StarSystemGenerator::generateSystem(uint32_t index) {
	// get this system's seed
	uint32_t y = index / 768; 
	uint32_t x = index % 768;
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

	// number of planets
	uint8_t planetcount = value/0.08;
	std::cerr << "Mass: " << mass << " Radius: " << radius << " Temp: " << temp << " Planets: " << (int)planetcount << std::endl;

	// for each planet
	// mass, radius, semimajor axis, eccentricity, orbital period, orbital plane tilt, 

	// number of moons
}
