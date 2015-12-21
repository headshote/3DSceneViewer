#pragma once

#include <vector>

#include "Renderable.h"
#include "DataStructures.hpp"
#include "rendering.h"

class Mesh : public Renderable
{
public:
	Mesh(GLfloat* vertices, GLuint numvertices, GLuint* meshTextures, GLuint numtextures, GLint isTransparent = 1);
	Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures);

	virtual ~Mesh();

protected:
	GLuint EBO = UINT_MAX;

	bool indexedMode = false;

	std::vector<Texture> textures;

	virtual void renderVAO(GLuint shaderProgram, GLuint VAO, GLuint numelements);
	virtual void batchRenderVAO(GLuint shaderProgram, GLuint VAO, GLuint numelements, GLuint numCalls, GLboolean dotMode);

private:
	void generateTangents(std::vector<glm::vec3>& tangents, std::vector<glm::vec3>& bitangents, const glm::vec3& pos1, const glm::vec3& pos2, const glm::vec3& pos3,
		const glm::vec2& uv1, const glm::vec2& uv2, const glm::vec2& uv3);
	void loadVertices(GLfloat* vertices, GLuint sizeOfVertices);
	void loadVertices(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices);

	void setUpMaterial(GLuint shaderprogram);
	void setModelTexture(GLuint shaderProgram, GLuint texture, const GLchar* shaderVar, GLuint textureId);
	void setModelMaterial(GLuint shaderProgram, GLfloat shininess, GLint isTransparent);

	Mesh(const Mesh&);
	Mesh& operator=(const Mesh&);
};

