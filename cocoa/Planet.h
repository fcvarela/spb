#ifndef _SPB_PLANET
#define _SPB_PLANET

#include <stdint.h>

#include <Factory.h>
#include <Shader.h>
#include <Node.h>

namespace SPB {
	class Planet {
	public:
		Planet();
		~Planet();

		void draw();

		Shader *terrainShader;
		Shader *atmosphereShader;

		double radius, atmosphere_radius;
		uint8_t maxlod;

		//TerrainQuadtree *quadtrees[6];

		Node *node;
	};
}

#endif