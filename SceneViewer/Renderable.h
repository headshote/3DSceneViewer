#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "rendering.h"

class Renderable
{
public:
	GLuint getVAO();

	virtual ~Renderable(){};

	void drawCall(GLuint shederprogram);

	void drawBatch(GLuint shederprogram, GLuint numDrawCalls);
protected:
	Renderable();

	GLuint VAO;
	GLuint VBO;

	GLuint nRenderingElemts;
	GLuint verticesSize;

	virtual void batchRenderVAO(GLuint shaderProgram, GLuint VAO, GLuint numelements, GLuint numCalls, GLboolean dotMode) = 0;
	virtual void renderVAO(GLuint shaderprogram, GLuint VAO, GLuint numelements) = 0;
private:

	Renderable(const Renderable&);
	Renderable& operator=(const Renderable&);
};

