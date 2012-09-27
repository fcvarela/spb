#include <Planet.h>
#include <Vector3.h>

namespace SPB {
	Planet::Planet() {
		// dimensions
		this->radius = 1738000.0;
		this->atmosphere_radius = this->radius * 1.025;

		// max lod
		this->maxlod = 15;

		this->terrainShader = new Shader("planet.glsl");
		this->atmosphereShader = new Shader("atmosphere.glsl");

		// quadtrees for cubefaces
		Vector3d center, dx, dy;

		// face 0 (top)
		center = Vector3d(0.0, 0.5, 0.0);
		dx = Vector3d(1.0, 0.0, 0.0);
		dy = Vector3d(0.0, 0.0, -1.0);
		TerrainQuadtree *qt1 = TerrainQuadtree(NULL, this->maxlod, 1, center, dx, dy);

		// face 1 (left)
		center = Vector3d(-0.5, 0.0, 0.0);
		dx = Vector3d(0.0, 0.0, 1.0);
		dy = Vector3d(0.0, 1.0, 0.0);
		TerrainQuadtree *qt2 = TerrainQuadtree(NULL, this->maxlod, 2, center, dx, dy);

		// face 2 (front)
		center = Vector3d(0.0, 0.0, 0.5);
		dx = Vector3d(1.0, 0.0, 0.0);
		dy = Vector3d(0.0, 1.0, 0.0);
		TerrainQuadtree *qt3 = TerrainQuadtree(NULL, this->maxlod, 3, center, dx, dy);

		// face 3 (right)
		center = Vector3d(0.5, 0.0, 0.0);
		dx = Vector3d(0.0, 0.0, -1.0);
		dy = Vector3d(0.0, 1.0, 0.0);
		TerrainQuadtree *qt4 = TerrainQuadtree(NULL, this->maxlod, 4, center, dx, dy);

		// face 4 (back)
		center = Vector3d(0.0, 0.0, -0.5);
		dx = Vector3d(-1.0, 0.0, 0.0);
		dy = Vector3d(0.0, 1.0, 0.0);
		TerrainQuadtree *qt5 = TerrainQuadtree(NULL, this->maxlod, 5, center, dx, dy);

		// face 5 (bottom)
		center = Vector3d(0.0, -0.5, 0.0);
		dx = Vector3d(1.0, 0.0, 0.0);
		dy = Vector3d(0.0, 0.0, 1.0);
		TerrainQuadtree *qt6 = TerrainQuadtree(NULL, this->maxlod, 6, center, dx, dy);

		// map them
		this->quadtrees[0] = qt1;
		this->quadtrees[1] = qt2;
		this->quadtrees[2] = qt3;
		this->quadtrees[3] = qt4;
		this->quadtrees[4] = qt5;
		this->quadtrees[5] = qt6;
	}

	Planet::~Planet() {
		for (uint8_t i=0; i<6; i++)
			delete this->quadtrees[i];

		delete this->terrainShader;
		delete this->atmosphereShader;
	}

	void Planet::draw() {
		terrainShader->bind();

		// prepare textures
		glUniform1i(glGetUniformLocation(this->terrainShader->program, "normalTexture"), 0);
		glUniform1i(glGetUniformLocation(this->terrainShader->program, "colorTexture"), 1);
		glUniform1i(glGetUniformLocation(this->terrainShader->program, "topoTexture"), 2);

		// prepare parent textures
		glUniform1i(glGetUniformLocation(this->terrainShader->program, "pnormalTexture"), 3);
		glUniform1i(glGetUniformLocation(this->terrainShader->program, "pcolorTexture"), 4);
		glUniform1i(glGetUniformLocation(this->terrainShader->program, "ptopoTexture"), 5);

		// near far
		glUniform1f(glGetUniformLocation(this->terrainShader->program, "far"), _far);
	}
}