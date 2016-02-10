
#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <iostream>

#include <memory>

#include <GL/glew.h> // Include glew to get all the required OpenGL headers

#include "utils.h"

class Shader
{
public:
	/**
		Constructor reads and builds the shaders from files
	*/
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath, const GLchar* geometryPath = nullptr);
	~Shader();

	/**
		Returns the shader program id
	*/
	GLuint getProgramId();

private:
	//no copy constructors and assignments
	Shader(const Shader&);
	Shader& operator=(const Shader&);

	// The program ID
	GLuint shaderProgram;

	GLuint compileShader(const GLchar** shaderSource, GLenum shaderType, const GLchar* shaderPath);
};

#endif	//SHADER_H

