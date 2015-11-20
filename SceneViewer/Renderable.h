#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Renderable
{
public:
	GLuint getVAO();

	Renderable();
	virtual ~Renderable(){};

	virtual void drawCall(GLuint shederprogram){}

	virtual void drawBatch(GLuint shederprogram, GLuint numDrawCalls){}

	virtual void dispose() {};
protected:
	GLuint VAO;
	GLuint VBO;

	GLuint nRenderingElemts;
	GLuint verticesSize;
};

