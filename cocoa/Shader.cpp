#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <Shader.h>

// needs serious error checking
Shader::Shader(const char *shader_file) {
	program = glCreateProgram();

	char *shader_source = loadShader(shader_file);
	if (shader_source == NULL)
		return;

	_vertex_shader = createShader(shader_source, GL_VERTEX_SHADER);
	_fragment_shader = createShader(shader_source, GL_FRAGMENT_SHADER);
	free(shader_source);
	link();
}

Shader::~Shader() {
	glDeleteShader(_vertex_shader);
	glDeleteShader(_fragment_shader);
	glDeleteProgram(program);
}

char *Shader::loadShader(const char *program_file) {
	FILE *fd;
	long len, r;
	char *program_source;

	if (!(fd = fopen(program_file, "r"))) {
		return NULL;
	}

	fseek(fd, 0, SEEK_END);
	len = ftell(fd)+1;
	fseek(fd, 0, SEEK_SET);

	if (!(program_source = (char *)malloc(len * sizeof(char)))) {
		return NULL;
	}
	memset(program_source, 0, len);
	r = fread(program_source, sizeof(char), len, fd);
	fclose(fd);
	return program_source;
}

GLuint Shader::createShader(const char *program_string, GLenum type) {
	if (strlen(program_string) < 1)
		return 0;

	char prefix[128] = "";
	switch (type) {
		case GL_VERTEX_SHADER: sprintf(prefix, "#define _VERTEX_"); break;
		case GL_FRAGMENT_SHADER: sprintf(prefix, "#define _FRAGMENT_"); break;
	}

	char *final_source = (char *)malloc(strlen(prefix)+1+strlen(program_string)+1);
	sprintf(final_source, "%s\n%s", prefix, program_string);
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, (const GLchar**)&final_source, NULL);
	glCompileShader(shader);
	free(final_source);

	// check compile status
	GLint compile_log;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_log);
	if (!compile_log) {
		// errored. get log
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &compile_log);
		char buffer[compile_log];
		glGetShaderInfoLog(shader, compile_log, NULL, buffer);
		std::string error_log(buffer);
		std::cerr << "--------------------------------" << std::endl;
		std::cerr << error_log << std::endl;
		std::cerr << "--------------------------------" << std::endl;

		return false;
	}

	// all went well
	glAttachShader(program, shader);
	return shader;
}

bool Shader::link() {
	glLinkProgram(program);
	// get status
	GLint link_log;
	glGetProgramiv(program, GL_LINK_STATUS, &link_log);
	if (!link_log) {
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &link_log);
		char buffer[link_log];
		glGetProgramInfoLog(program, link_log, NULL, buffer);
		std::string error_log(buffer);
		std::cerr << "--------------------------------" << std::endl;
		std::cerr << error_log << std::endl;
		std::cerr << "--------------------------------" << std::endl;

		return false;
	}
	return true;
}

void Shader::bind() {glUseProgram(program);}
void Shader::unbind() {glUseProgram(0);}
