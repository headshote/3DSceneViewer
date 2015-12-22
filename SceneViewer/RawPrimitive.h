#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Renderable.h"

class RawPrimitive : public Renderable
{
public:
	RawPrimitive(GLfloat* vertices, const GLuint sizeOfVertices, const glm::vec3 primitiveColor);
	RawPrimitive(GLfloat* vertices, const GLuint sizeOfVertices);

	virtual ~RawPrimitive();

	virtual GLuint getVAO();

protected:
	virtual void render(const GLuint shaderprogram);
	virtual void batchRender(const GLuint shaderProgram, const GLuint numCalls);

private:
	GLuint VAO;
	GLuint VBO;

	GLuint nRenderingElemts;
	GLuint verticesSize;

	//disabled copy constructor and assignment
	RawPrimitive(const RawPrimitive&);
	RawPrimitive& operator=(const RawPrimitive&);

	glm::vec3 color;

	void setShaderColor(const GLuint shaderProgram, const glm::vec3& primitiveColor);

	GLuint loadRawVertices(GLfloat* vertices);
	GLuint load2DTexturedVertices(GLfloat* vertices); 

};

