#include <GL/glew.h>
#include <GL/glfw.h>
#include <TerrainQuadtree.h>
#include <GameSceneManager.h>
#include <TerrainLoader.h>
#include <Planet.h>

#include <list>

TerrainQuadtree::TerrainQuadtree(TerrainQuadtree *parent, Planet *planet, uint16_t maxlod, uint8_t index, Vector3d &center, Vector3d &dx, Vector3d &dy) {
	this->maxlod = maxlod;
	this->index = index;

	this->parent = NULL;
	if (parent != NULL) {
		this->parent = parent;
		this->fullAddress = parent->fullAddress * 10 + this->index;
	} else
		this->fullAddress = this->index;

	this->planet = planet;
	this->position = center;
	this->dx = dx;
	this->dy = dy;

	this->gridSize = 32;
	this->gridSizep1 = this->gridSize+1;

	this->textureBorder = 2;
	this->textureSize = 256 + 2*this->textureBorder;

	this->centerHeight = 0.0;

	for (uint8_t i=0; i<4; i++) {
		this->children[i] = NULL;
		this->sphere[i] = NULL;
	}

	this->distanceToCamera = 0.0;
	this->morphWeight = 1.0;

	this->lastDrawn = glfwGetTime();

	// vbos
	this->positionBufferObject = 0;
	this->texturecoordBufferObject = 0;
	this->indexBufferObject = 0;
	this->skirtIndexBufferObject = 0;

	// shaders
	this->generatorShader = NULL;
	this->generatorShaderN = NULL;
	this->generatorShaderC = NULL;

	if (parent != NULL) {
		// VBOS are shared
		this->texturecoordBufferObject = parent->texturecoordBufferObject;
		this->indexBufferObject = parent->indexBufferObject;
		this->skirtIndexBufferObject = parent->skirtIndexBufferObject;

		// and so are shaders
		this->generatorShader = parent->generatorShader;
		this->generatorShaderN = parent->generatorShaderN;
		this->generatorShaderC = parent->generatorShaderC;

		this->framebuffer = parent->framebuffer;
	}

	this->positionTexture = NULL;
	this->topoTexture = NULL;
	this->normalTexture = NULL;
	this->colorTexture = NULL;

	// mark us as not ready
	this->ready = false;

	// init
	this->generateVertices();
	TerrainLoader *loader = getTerrainLoader();
	loader->enqueue(this);
}

TerrainQuadtree::~TerrainQuadtree() {
	// nada
}

void TerrainQuadtree::init() {
	this->finishVertices();
	this->generateTextures();
}

void TerrainQuadtree::generateTextures() {
	// prepare viewport and projection
	//glDisable(GL_DEPTH_TEST);
	
	// do not setup the proj more than once
	if (this->parent == NULL) {
		glViewport(0, 0, this->textureSize, this->textureSize);
		glMatrixMode(GL_PROJECTION);
		glPolygonMode(GL_FRONT, GL_FILL);
		glLoadIdentity();
		glOrtho(0.0, (double)this->textureSize, 0.0, (double)this->textureSize, 0.0, 1.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glTranslated(0.0, 0.0, -1.0);

		generationScreenList = glGenLists(1);
		glNewList(generationScreenList, GL_COMPILE);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(0.0, 0.0, 0.0);
		glTexCoord2f(1.0, 0.0);
		glVertex3f(this->textureSize, 0.0, 0.0);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(this->textureSize, this->textureSize, 0.0);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(0.0, this->textureSize, 0.0);
		glEnd();
		glEndList();
	}

	// generate topography
	glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->topoTexture->id, 0);
	this->positionTexture->bind(GL_TEXTURE0);
	
	this->generatorShader->bind();
	glUniform1i(glGetUniformLocation(this->generatorShader->program, "positionTexture"), 0);
	glCallList(generationScreenList);
	this->generatorShader->unbind();
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// generate normals
	//glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->normalTexture->id, 0);
	this->topoTexture->bind(GL_TEXTURE1);

	this->generatorShaderN->bind();
	glUniform1f(glGetUniformLocation(this->generatorShaderN->program, "size"), (float)this->textureSize);
	glUniform1f(glGetUniformLocation(this->generatorShaderN->program, "radius"), this->planet->radius);
	glUniform1i(glGetUniformLocation(this->generatorShaderN->program, "topoTexture"), 1);
	glUniform1i(glGetUniformLocation(this->generatorShaderN->program, "positionTexture"), 0);
	glCallList(generationScreenList);
	this->generatorShaderN->unbind();
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// generate colors
	//glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->colorTexture->id, 0);
	this->generatorShaderC->bind();
	glUniform1i(glGetUniformLocation(this->generatorShaderC->program, "topoTexture"), 1);
	glCallList(generationScreenList);
	this->generatorShaderC->unbind();
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//glEnable(GL_DEPTH_TEST);
	this->ready = true;
}

void TerrainQuadtree::generateVertices() {
	// vertices
	uint64_t coord_count = this->gridSizep1*this->gridSizep1*3 + 3;
	this->vertices = (double *)malloc(sizeof(double) * coord_count);
	this->buildQuadtreeElementd(this->vertices, this->gridSize, 0, 0);

	// positions
	coord_count = (this->gridSizep1+2)*(this->gridSizep1+2)*3;
	this->positionTextureContent = (float *)malloc(sizeof(float) * coord_count);
	this->buildQuadtreeElementf(this->positionTextureContent, this->gridSize, 1, 1);

	uint32_t u = 0;
	uint32_t v = 0;
	this->topleft = Vector3d(
		vertices[u*this->gridSizep1*3+v*3+0],
		vertices[u*this->gridSizep1*3+v*3+1],
		vertices[u*this->gridSizep1*3+v*3+2]);

	v = this->gridSize;
	this->topright = Vector3d(
		vertices[u*this->gridSizep1*3+v*3+0],
		vertices[u*this->gridSizep1*3+v*3+1],
		vertices[u*this->gridSizep1*3+v*3+2]);

	u = this->gridSize;
	v = 0;
	this->botleft = Vector3d(
		vertices[u*this->gridSizep1*3+v*3+0],
		vertices[u*this->gridSizep1*3+v*3+1],
		vertices[u*this->gridSizep1*3+v*3+2]);	

	v = this->gridSize;
	this->botright = Vector3d(
		vertices[u*this->gridSizep1*3+v*3+0],
		vertices[u*this->gridSizep1*3+v*3+1],
		vertices[u*this->gridSizep1*3+v*3+2]);

	this->vertices[this->gridSizep1*this->gridSizep1*3 + 0] = position.x() * 0.1;
	this->vertices[this->gridSizep1*this->gridSizep1*3 + 1] = position.y() * 0.1;
	this->vertices[this->gridSizep1*this->gridSizep1*3 + 2] = position.z() * 0.1;

	this->sideLength = (topleft - botleft).length();
	this->diagonalLength = (topleft - botright).length();
}

void TerrainQuadtree::finishVertices() {
	this->positionTexture = new Texture(this->textureSize, false);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, this->gridSizep1+2, this->gridSizep1+2, 0, GL_RGB, GL_FLOAT, this->positionTextureContent);
	free(this->positionTextureContent);

	uint16_t coord_count = this->gridSizep1*this->gridSizep1*3 + 3;
	glGenBuffers(1, &this->positionBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, this->positionBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(double)*coord_count, this->vertices, GL_STATIC_DRAW);
	free(this->vertices);

	// build our indexes
	if (this->indexBufferObject == 0) {
		uint16_t added_indexes = 0;
		std::list<uint16_t> v_indexes;
	
		for (uint8_t y=0; y<this->gridSize; y++) {
			for (uint8_t x=0; x<this->gridSizep1; x++) {
				v_indexes.push_back(y * this->gridSizep1 + x);
				v_indexes.push_back((y+1) * this->gridSizep1 + x);

				added_indexes += 2;
				// degerate this srip?
				if (added_indexes % this->gridSizep1*2 == 0) {
					// last vertex
					v_indexes.push_back((y+1) * this->gridSizep1 + x);

					// next vertex twice
					v_indexes.push_back((y+1) * this->gridSizep1);
					v_indexes.push_back((y+1) * this->gridSizep1);

					// add next vertex
					v_indexes.push_back((y+2) * this->gridSizep1);
				}
			}
		}

		this->indexes = (uint16_t *)malloc(sizeof(uint16_t) * v_indexes.size());
		uint16_t j=0;
		for (std::list<uint16_t>::iterator i=v_indexes.begin(); i!=v_indexes.end(); ++i) {
			this->indexes[j] = *i;
			j++;
		}

		// store in VBO
		glGenBuffers(1, &this->indexBufferObject);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->indexBufferObject);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t)*v_indexes.size(), this->indexes, GL_STATIC_DRAW);
		free(this->indexes);
	}

	// skirts missing for now
	
	// build our texture coordinates
	std::list<double> v_texcoords;
	if (this->texturecoordBufferObject == 0) {
		for (uint16_t y=0; y<this->gridSizep1; y++) {
			for (uint16_t x=0; x<this->gridSizep1; x++) {
				double cx = (double)x / (double)this->gridSize;
				double cy = (double)(this->gridSize-y) / (double)this->gridSize;

				// coords are 0,1. map to texelstep, 1-texelstep
				double texelstep = 1.0/23.0;
				double steprange = 1.0 - texelstep*2.0;

				cx = texelstep + cx*steprange;
				cy = texelstep + cy*steprange;

				v_texcoords.push_back(cx);
				v_texcoords.push_back(cy);
			}
		}
		v_texcoords.push_back(0.5);
		v_texcoords.push_back(0.5);

		this->texcoords = (double *)malloc(sizeof(double) * v_texcoords.size());
		uint16_t j=0;
		for (std::list<double>::iterator i=v_texcoords.begin(); i!=v_texcoords.end(); ++i) {
			this->texcoords[j] = *i;
			j++;
		}

		// store in VBO
		glGenBuffers(1, &this->texturecoordBufferObject);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->texturecoordBufferObject);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(double)*v_texcoords.size(), this->texcoords, GL_STATIC_DRAW);
		free(this->texcoords);

	}

	// initialize our textures
	this->topoTexture = new Texture(this->textureSize, true);
	this->colorTexture = new Texture(this->textureSize, true);
	this->normalTexture = new Texture(this->textureSize, true);

	if (this->generatorShader == NULL) {
		this->generatorShader = new Shader("data/shaders/planet-generator.glsl");
		this->generatorShaderN = new Shader("data/shaders/planet-generator-normals.glsl");
		this->generatorShaderC = new Shader("data/shaders/planet-generator-colors.glsl");
	}

	this->generateTextures();
}

void TerrainQuadtree::analyse(double weight = 0.0) {
	if (this->ready == false)
		return;

	// do we need to draw our children?
	Camera *camera = getGameSceneManager()->camera;
	double d1 = (camera->position - this->position).length() * planet->radius;
	double d2 = (camera->position - this->topleft).length() * planet->radius;
	double d3 = (camera->position - this->topright).length() * planet->radius;
	double d4 = (camera->position - this->botleft).length() * planet->radius;
	double d5 = (camera->position - this->topright).length() * planet->radius;
	double mindistance = fmin(d1, fmin(d2, fmin(d3, fmin(d4, d5))));
	this->distanceToCamera = mindistance;

	double near = this->diagonalLength * 1.01;
	double far = this->diagonalLength * 1.15;

	if (this->maxlod > 0 && this->distanceToCamera <= far) {
		if (this->children[0] != NULL && this->children[1] != NULL && this->children[2] != NULL && this->children[3] != NULL) {
			uint8_t readycount = 0;
			for (uint8_t i=0; i<4; i++) {
				if (this->children[i]->ready == true)
					readycount++;
			}
			if (readycount == 4) {
				// we can and need to draw our children.
				// morph?
				double factor = 0.0;
				if (mindistance >= near) {
					// compute factor for morph
					factor = (mindistance - near) / (far-near);
					if (factor > 1.0) factor = 1.0;
					if (factor < 0.0) factor = 0.0;
					factor = factor * factor * (3.0 - 2.0*factor);
				}
				// recursively analyse our children
				this->children[0]->analyse(factor);
				this->children[1]->analyse(factor);
				this->children[2]->analyse(factor);
				this->children[3]->analyse(factor);

				// make sure we break here
				return;
			}
		} else {
			// init our children. we need them
		}
	}

	this->morphWeight = weight;
	this->draw();
}

void TerrainQuadtree::draw() {
	glUniform1f(glGetUniformLocation(this->planet->surfaceShader->program, "weight"), this->morphWeight);
	glUniform1i(glGetUniformLocation(this->planet->surfaceShader->program, "index"), this->index);
	glUniform1f(glGetUniformLocation(this->planet->surfaceShader->program, "texturesize"), this->textureSize-this->textureBorder);

	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, this->positionBufferObject);
	glVertexPointer(3, GL_DOUBLE, 0, 0);

	glEnableClientState(GL_INDEX_ARRAY);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->indexBufferObject);

	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, this->texturecoordBufferObject);
	glTexCoordPointer(2, GL_DOUBLE, 0, 0);

	this->normalTexture->bind(GL_TEXTURE0);
	this->colorTexture->bind(GL_TEXTURE1);
	this->topoTexture->bind(GL_TEXTURE2);

	if (this->parent != NULL) {
		this->parent->normalTexture->bind(GL_TEXTURE3);
		this->parent->colorTexture->bind(GL_TEXTURE4);
		this->parent->topoTexture->bind(GL_TEXTURE5);
	}

	uint16_t indexcount = (this->gridSizep1*this->gridSize*2) + (this->gridSize*4);
	glDrawElements(GL_TRIANGLE_STRIP, indexcount, GL_UNSIGNED_SHORT, 0);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_INDEX_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	this->lastDrawn = glfwGetTime();
}

void TerrainQuadtree::initChildren() {}

void TerrainQuadtree::buildQuadtreeElementd(double *mesh, uint8_t size, uint8_t reverse, uint8_t border) {
	double u, v, gridsizeover2, gsize, upos, vpos;
	Vector3d coord, dxovergridsize, dyovergridsize;

	// temp stuff
	gsize = size + 1.0 + (border * 2.0);
	dxovergridsize = dx/(double)size;
	dyovergridsize = dy/(double)size;
	gridsizeover2 = size/2.0;

	for (u=0; u<gsize; u++) {
		for (v=0; v<gsize; v++) {
			// build
			coord = position + dxovergridsize * ((v-border)-gridsizeover2) + dyovergridsize * (gridsizeover2 - (u-border));

			// normalize
			coord.normalize();
			
			if (reverse == 1)
				upos = (gsize-1.0-u)*(gsize)*3.0;
			else
				upos = gsize * u * 3.0;
			vpos = v*3.0;

			mesh[(int16_t)upos + (int16_t)vpos + 0] = coord.x();
			mesh[(int16_t)upos + (int16_t)vpos + 1] = coord.y();
			mesh[(int16_t)upos + (int16_t)vpos + 2] = coord.z();
		}
	}
}

void TerrainQuadtree::buildQuadtreeElementf(float *mesh, uint8_t size, uint8_t reverse, uint8_t border) {
	double u, v, gridsizeover2, gsize, upos, vpos;
	Vector3d coord, dxovergridsize, dyovergridsize;

	// temp stuff
	gsize = size + 1.0 + (border * 2.0);
	dxovergridsize = dx/(double)size;
	dyovergridsize = dy/(double)size;
	gridsizeover2 = size/2.0;

	for (u=0; u<gsize; u++) {
		for (v=0; v<gsize; v++) {
			// build
			coord = position + dxovergridsize * ((v-border)-gridsizeover2) + dyovergridsize * (gridsizeover2 - (u-border));

			// normalize
			coord.normalize();
			
			if (reverse == 1)
				upos = (gsize-1.0-u)*(gsize)*3.0;
			else
				upos = gsize * u * 3.0;
			vpos = v*3.0;

			mesh[(int16_t)upos + (int16_t)vpos + 0] = coord.x();
			mesh[(int16_t)upos + (int16_t)vpos + 1] = coord.y();
			mesh[(int16_t)upos + (int16_t)vpos + 2] = coord.z();
		}
	}
}
