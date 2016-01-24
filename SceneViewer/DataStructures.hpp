#ifndef DATA_STRUCTURES
#define DATA_STRUCTURES

#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 textureCoords;
	glm::vec3 tangent;
	glm::vec3 bitangent;
};

struct Texture {
	GLuint id;
	std::string type;
	aiString name;
	GLint transparency;
};

struct TextureData{
	std::string path;
	std::string typeName;
};

#endif