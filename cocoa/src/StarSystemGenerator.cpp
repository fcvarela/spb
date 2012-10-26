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

	// star mass, radius, temperature
	// mass interval 0.1 to 1500 solar masses
	double starmass = 0.1 + ((value+1.0)/2.0) * 1500.0;
	if (starmass < 0.0)
		std::cerr << value << std::endl;

	// number of planets

	// for each planet
	// mass, radius, semimajor axis, eccentricity, orbital period, orbital plane tilt, 

	// number of moons
}
