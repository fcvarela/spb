#ifndef __SHADER
#define __SHADER

#include <GL/glfw.h>

class Shader {
private:
	GLint _vertex_shader, _fragment_shader/*, _geometry_shader*/;

	char *loadShader(const char *program_file);
	GLuint createShader(const char *program_string, GLenum type);
	bool link();

public:
	GLint program;

	Shader(const char *shader_file);
	~Shader();

	void bind();
	void unbind();
};

#endif
