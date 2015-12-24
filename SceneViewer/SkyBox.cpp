#include "SkyBox.h"

using namespace renderables;

SkyBox::SkyBox(const std::vector<std::string>& textures_faces, GLboolean gammacorrection) : useGammaCorrection(gammacorrection)
{
	createCubeMap(textures_faces);
	std::vector<GLfloat> skyboxVertices = {
		// Positions          
		-1.0f, 1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,

		-1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,

		-1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f
	};
	generateCubeMapVAO(skyboxVertices);
}


SkyBox::~SkyBox()
{
	glDeleteTextures(1, &cubemapID);
	glDeleteBuffers(1, &skyboxVBO);
	glDeleteVertexArrays(1, &skyboxVAO);
}


GLuint SkyBox::getVAO()
{
	return skyboxVAO;
}

void SkyBox::render(const GLuint shaderprogram)
{
	glUseProgram(shaderprogram);
	glDepthMask(GL_FALSE);

	glBindVertexArray(skyboxVAO);

	//activate cubemap
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);

	glDrawArrays(GL_TRIANGLES, 0, nrenderingElements);
	glBindVertexArray(0);
	glDepthMask(GL_TRUE);
}

void SkyBox::batchRender(const GLuint shaderProgram, const GLuint numCalls)
{

}

void SkyBox::createCubeMap(const std::vector<std::string>& textures_faces)
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	//Set texture for each face of a cubemap
	int width, height;
	unsigned char* image;
	for (GLuint i = 0; i < textures_faces.size(); i++)
	{
		image = SOIL_load_image(textures_faces[i].c_str(), &width, &height, 0, SOIL_LOAD_RGB);
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0,
			useGammaCorrection ? GL_SRGB : GL_RGB,
			width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image
			);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	cubemapID = textureID;
}

void SkyBox::generateCubeMapVAO(const std::vector<GLfloat>& skyboxVertices)
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
	glBufferData(GL_ARRAY_BUFFER, skyboxVertices.size()*sizeof(GLfloat), &skyboxVertices[0], GL_STATIC_DRAW);

	// 3. Then set the vertex attributes pointers
	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, entriesPerVertex * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// 4. Unbind VAO 
	glBindVertexArray(0);

	skyboxVAO =  VAO;
	skyboxVBO = VBO;
	nrenderingElements = skyboxVertices.size();
}


