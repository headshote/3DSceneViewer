
#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <memory>

#include <GL/glew.h> // Include glew to get all the required OpenGL headers

class Shader
{
public:
	/**
		Returns the shader program id
	*/
	GLuint getProgramId();

	/**
		 Constructor reads and builds the shaders from files
	*/	
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath, const GLchar* geometryPath = nullptr);
	~Shader();
private:
	// The program ID
	GLuint shaderProgram;

	std::string* readFile(const GLchar* filePath);
	GLuint compileShader(const GLchar** shaderSource, GLenum shaderType, const GLchar* shaderPath);
};

#endif	//SHADER_H

