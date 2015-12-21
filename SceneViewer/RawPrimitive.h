#pragma once

#include "Renderable.h"

class RawPrimitive : public Renderable
{
public:
	RawPrimitive(GLfloat* vertices, GLuint verticesSize, glm::vec3 primitiveColor);
	RawPrimitive(GLfloat* vertices, GLuint verticesSize);

	virtual ~RawPrimitive();

protected:
	glm::vec3 color;

	void setShaderColor(GLuint shaderProgram, const glm::vec3& primitiveColor);

	virtual void renderVAO(GLuint shaderProgram, GLuint VAO, GLuint numelements);
	virtual void batchRenderVAO(GLuint shaderProgram, GLuint VAO, GLuint numelements, GLuint numCalls, GLboolean dotMode);

	GLuint loadRawVertices(GLfloat* vertices, GLuint sizeOfVertices);
	GLuint load2DTexturedVertices(GLfloat* vertices, GLuint sizeOfVertices); 

private:
	RawPrimitive(const RawPrimitive&);
	RawPrimitive& operator=(const RawPrimitive&);
};

