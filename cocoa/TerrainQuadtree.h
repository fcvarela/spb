#ifndef __TERRAINQUADTREE
#define __TERRAINQUADTREE

#include <stdint.h>
#include <Node.h>
#include <Shader.h>
#include <Texture.h>
#include <Vector.h>

class Planet;

class TerrainQuadtree : public Node {
public:
	TerrainQuadtree(TerrainQuadtree *parent, Planet *planet, uint16_t maxlod, uint8_t index, Vector3d &center, Vector3d &dx, Vector3d &dy);
	~TerrainQuadtree();

	void buildQuadtreeElementd(double *mesh, uint8_t size, uint8_t reverse, uint8_t border);
	void buildQuadtreeElementf(float *mesh, uint8_t size, uint8_t reverse, uint8_t border);

	uint16_t maxlod;
	uint8_t index;
	uint64_t fullAddress;

	TerrainQuadtree *parent;
	Planet *planet;

	Vector3d dx;
	Vector3d dy;

	uint16_t gridSize;
	uint16_t gridSizep1;

	uint8_t textureBorder;
	uint16_t textureSize;

	double centerHeight;
	double sideLength;
	double diagonalLength;

	double sphere[4];
	Vector3d box[8];

	TerrainQuadtree *children[4];

	double distanceToCamera;
	double morphWeight;

	double lastDrawn;

	GLuint positionBufferObject;
	GLuint texturecoordBufferObject;
	GLuint indexBufferObject;
	GLuint skirtIndexBufferObject;

	Shader *generatorShader;
	Shader *generatorShaderN;
	Shader *generatorShaderC;

	Texture *positionTexture;
	Texture *topoTexture;
	Texture *normalTexture;
	Texture *colorTexture;

	// aux
	double *vertices;
	uint16_t *indexes;
	uint16_t *skirtIndexes;
	double *texcoords;
	float *positionTextureContent;

	// corners
	Vector3d topleft, topright, botleft, botright;

	GLuint framebuffer;

	bool ready;

	void generateTextures();
	void generateVertices();
	void finishVertices();

	void analyse(double weight);
	void draw();

	void initChildren();
};

#endif
