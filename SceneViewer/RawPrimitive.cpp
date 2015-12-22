#include "RawPrimitive.h"

/**
	The case of colored 3d primitive (only array of 3 position coordinates)
*/
RawPrimitive::RawPrimitive(GLfloat* vertices, const GLuint sizeOfVertices, const glm::vec3 primitiveColor) : verticesSize(sizeOfVertices), color(primitiveColor)
{
	VAO = loadRawVertices(vertices);
}

/**
	The case of textured 2d primitive (2 positions, 2 texture coordinates)
*/
RawPrimitive::RawPrimitive(GLfloat* vertices, const GLuint sizeOfVertices) : verticesSize(sizeOfVertices)
{
	VAO = load2DTexturedVertices(vertices);
}

RawPrimitive::~RawPrimitive()
{
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
}

GLuint RawPrimitive::getVAO()
{
	return VAO;
}

/**
	Loads vertices from a C++ array into OpenGL Vertex Array Object,
	which can be used for later rendering, no texture coordinates, or vertex normals are used here,
	just the raw positions
*/
GLuint RawPrimitive::loadRawVertices(GLfloat* vertices)
{
	GLuint entriesPerVertex = 3;

	GLuint VBO;
	glGenBuffers(1, &VBO);
	GLuint VAO;
	glGenVertexArrays(1, &VAO);

	// 1. Bind Vertex Array Object
	glBindVertexArray(VAO);

	// 2. Copy our vertices array in a vertex buffer for OpenGL to use
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, verticesSize, vertices, GL_STATIC_DRAW);

	// 3. Then set the vertex attributes pointers
	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, entriesPerVertex * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// 4. Unbind VAO 
	glBindVertexArray(0);

	nRenderingElemts = (verticesSize / sizeof(GLfloat)) / entriesPerVertex;

	return VAO;
}

/**
	Loads vertices from a C++ array into OpenGL Vertex Array Object,
	which can be used for later rendering, no vertex normals are used here,
	just the raw positions, texture coordinates
*/
GLuint RawPrimitive::load2DTexturedVertices(GLfloat* vertices)
{
	GLuint entriesPerVertex = 4;

	GLuint VBO;
	glGenBuffers(1, &VBO);
	GLuint VAO;
	glGenVertexArrays(1, &VAO);

	// 1. Bind Vertex Array Object
	glBindVertexArray(VAO);

	// 2. Copy our vertices array in a vertex buffer for OpenGL to use
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, verticesSize, vertices, GL_STATIC_DRAW);

	// 3. Then set the vertex attributes pointers
	// Position attribute
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, entriesPerVertex * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, entriesPerVertex * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)) );
	glEnableVertexAttribArray(1);

	// 4. Unbind VAO 
	glBindVertexArray(0);

	nRenderingElemts = (verticesSize / sizeof(GLfloat)) / entriesPerVertex;

	return VAO;
}

void RawPrimitive::setShaderColor(const GLuint shaderProgram, const glm::vec3& primitiveColor)
{
	//set color
	glUniform3f(glGetUniformLocation(shaderProgram, "lsColor"), primitiveColor.x, primitiveColor.y, primitiveColor.z);
}

/**
	Renders raw (no textures, or normals) primitive as a wireframe
*/
void RawPrimitive::render(const GLuint shaderProgram)
{
	setShaderColor(shaderProgram, color);

	//Wireframe on
	GLint polygonMode;
	glGetIntegerv(GL_POLYGON_MODE, &polygonMode);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, nRenderingElemts);

	//Return to previous rendering mode
	glPolygonMode(GL_FRONT_AND_BACK, polygonMode);

	glBindVertexArray(0);
}

void RawPrimitive::batchRender(const GLuint shaderProgram, const GLuint numCalls)
{

}
