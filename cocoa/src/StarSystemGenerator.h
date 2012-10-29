#ifndef __STARSYSTEMGENERATOR
#define __STARSYSTEMGENERATOR

#include <noise/noise.h>
#include <noiseutils.h>

class StarSystem;

class StarSystemGenerator {
public:
	module::Perlin generatorModule;
	utils::NoiseMap systemMap;
	utils::NoiseMapBuilderPlane systemMapBuilder;

	StarSystemGenerator();
	~StarSystemGenerator();

	void generateSystem(StarSystem *system);
};

#endif
