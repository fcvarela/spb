#ifndef __TEXTURE
#define __TEXTURE

#include <stdint.h>
#include <GL/glfw.h>

class Texture {
public:
	Texture(uint16_t size, bool init=true);
	~Texture();

	uint16_t size;
	GLuint id;

	void bind(GLuint textureUnit);
	void unbind();
};

#endif
