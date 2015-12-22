#include "Shader.h"

Shader::Shader(const GLchar* vertexPath, const GLchar* fragmentPath, const GLchar* geometryPath)
{	
	GLuint vertex;
	GLuint fragment;
	GLuint geometry;

	GLboolean geometryAvailable = geometryPath != nullptr;

	//read shader code into a string from the files
	std::shared_ptr<std::string> vsting( readFile(vertexPath) );
	std::shared_ptr<std::string> fsting( readFile(fragmentPath) );
	const GLchar* vShaderCode = vsting->c_str();
	const GLchar* fShaderCode = fsting->c_str();

	if (geometryAvailable)
	{
		std::shared_ptr<std::string> gsting(readFile(geometryPath));
		const GLchar* gShaderCode = gsting->c_str();
		geometry = compileShader(&gShaderCode, GL_GEOMETRY_SHADER, geometryPath);
	}

	// 2. Compile shaders
	vertex = compileShader(&vShaderCode, GL_VERTEX_SHADER, vertexPath);
	fragment = compileShader(&fShaderCode, GL_FRAGMENT_SHADER, fragmentPath);	

	// Shader Program
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertex);
	glAttachShader(shaderProgram, fragment);
	if (geometryAvailable)	glAttachShader(shaderProgram, geometry);
	glLinkProgram(shaderProgram);
	// Print linking errors if any
	GLint success;
	GLchar infoLog[512];
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	// Delete the shaders as they're linked into our program now and no longer necessery
	glDeleteShader(vertex);
	glDeleteShader(fragment);
	if (geometryAvailable)	glDeleteShader(geometry);
}

Shader::~Shader()
{
	glDeleteShader(shaderProgram);
}

GLuint Shader::getProgramId()
{
	return shaderProgram;
}

std::string* Shader::readFile(const GLchar* filePath)
{
	// 1. Retrieve the vertex/fragment source code from filePath
	std::string* shaderCode = nullptr;
	std::ifstream shaderFile;

	// ensures ifstream objects can throw exceptions:
	shaderFile.exceptions(std::ifstream::badbit);

	try
	{
		std::stringstream shaderStream;

		// Open file
		shaderFile.open(filePath);
		// Read file's buffer contents into streams
		shaderStream << shaderFile.rdbuf();
		// close file handlers
		shaderFile.close();
		// Convert stream into GLchar array
		shaderCode = new std::string( shaderStream.str() );
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}

	return shaderCode;
}

GLuint Shader::compileShader(const GLchar** shaderSource, GLenum shaderType, const GLchar* shaderPath)
{
	GLuint shader;
	shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, shaderSource, NULL);
	glCompileShader(shader);

	//Checking for shader compile-time errors
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		std::string shadertype = (shaderType == GL_VERTEX_SHADER) ? "VERTEX" : (shaderType == GL_FRAGMENT_SHADER ? "FRAGMENT" : "GEOMETRY");
		std::cout << "ERROR::SHADER::" << shadertype << "::COMPILATION_FAILED\nShader file: " << shaderPath << "\n" << infoLog << std::endl;
	}

	return shader;
}