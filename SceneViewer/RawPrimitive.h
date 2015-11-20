#pragma once

#include "Renderable.h"

class RawPrimitive : public Renderable
{
public:
	RawPrimitive(GLfloat* vertices, GLuint verticesSize, glm::vec3 primitiveColor);
	RawPrimitive(GLfloat* vertices, GLuint verticesSize);
	virtual ~RawPrimitive();

	void drawCall(GLuint shederprogram);

	void dispose();
protected:
	glm::vec3 color;

	void renderRawPrimitive(GLuint shaderProgram, GLuint VAO, GLuint numelements, const glm::vec3& primitiveColor);
	GLuint loadRawVertices(GLfloat* vertices, GLuint sizeOfVertices);
	GLuint load2DTexturedVertices(GLfloat* vertices, GLuint sizeOfVertices);
};

