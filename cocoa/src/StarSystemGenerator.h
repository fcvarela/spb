#ifndef __STARSYSTEMGENERATOR
#define __STARSYSTEMGENERATOR

#include <noise/noise.h>
#include <noiseutils.h>

class StarSystemGenerator {
public:
	module::Perlin generatorModule;
	utils::NoiseMap systemMap;
	utils::NoiseMapBuilderPlane systemMapBuilder;

	StarSystemGenerator();
	~StarSystemGenerator();

	void generateSystem(uint32_t index);

};

#endif
