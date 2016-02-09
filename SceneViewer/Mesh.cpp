#include "Mesh.h"

using namespace renderables;

Mesh::Mesh(GLfloat* vertices, GLuint numvertices, GLuint* meshTextures, GLuint numtextures, GLint isTransparent) : verticesSize(numvertices)
{
	loadVertices(vertices);

	for (GLuint i = 0; i < numtextures; i++)
	{
		Texture texture;
		texture.id = meshTextures[i];

		if (i == 0)
			texture.type = "texture_diffuse";
		else if (i == 1)
			texture.type = "texture_specular";
		else if ( i == 2 )
			texture.type = "texture_normal";
		else if (i == 3)
			texture.type = "texture_depth";

		texture.name = "";
		texture.transparency = isTransparent;
		textures.push_back( texture );
	}
}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> meshTextures) : verticesSize(vertices.size() * sizeof(Vertex)), textures(meshTextures)
{
	loadVertices(vertices, indices);
}

Mesh::~Mesh()
{
	if (EBO < (GLuint)-1)
		glDeleteBuffers(1, &EBO);
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
}

GLuint Mesh::getVAO()
{
	return VAO;
}

/**
	Loads vertices from a C++ float array (3 positions, 3 normals, 2 textrure coords)
	into OpenGL Vertex Array Object, which can be used for later rendering (VAO)
*/
void Mesh::loadVertices(GLfloat* vertices)
{
	GLuint entriesPerVertex = 8;

	glGenBuffers(1, &VBO);
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
	//Normals attribute
	glVertexAttribPointer(1,	//that's at location = 1 in the second version of the vertex shader
		3,	//3 values, vec3
		GL_FLOAT,	//Data type
		GL_FALSE,	//normalize(-1...1)?
		entriesPerVertex * sizeof(GLfloat),	//Stride: we have 8 floats there ( 3 pos, 3 normals, 2 texture coords )
		(GLvoid*)(3 * sizeof(GLfloat)));	//Normals start with the 6th float in shadedVertices array
	glEnableVertexAttribArray(1);
	//Texture attribute
	glVertexAttribPointer(2,	//that's at location = 2 in the second version of the vertex shader
		2,	//2 values, vec2
		GL_FLOAT,	//Data type
		GL_FALSE,	//normalize(-1...1)?
		entriesPerVertex * sizeof(GLfloat),	//Stride: we have 8 floats there ( 3 pos, 3 normals, 2 texture coords )
		(GLvoid*)(6 * sizeof(GLfloat)));	//Text coords start with the fourth float in shadedVertices array
	glEnableVertexAttribArray(2);

	//calculate tangent and bitanget vectors per triangle, and shove them into attributes 4 and 5 of the vao
	GLuint totalNVertexVars = verticesSize / sizeof(GLfloat);
	GLuint nVertices = totalNVertexVars / 8;
	std::vector<glm::vec3> tangents;
	std::vector<glm::vec3> bitangents;
	for (GLuint i = 0; i < totalNVertexVars; i += entriesPerVertex * 3)
	{
		glm::vec3 pos1(vertices[i], vertices[i + 1], vertices[i + 2]);
		glm::vec3 pos2(vertices[i + entriesPerVertex], vertices[i + entriesPerVertex + 1], vertices[i + entriesPerVertex + 2]);
		glm::vec3 pos3(vertices[i + entriesPerVertex * 2], vertices[i + entriesPerVertex * 2 + 1], vertices[i + entriesPerVertex * 2 + 2]);

		glm::vec2 uv1(vertices[i + 6], vertices[i + 7]);
		glm::vec2 uv2(vertices[i + entriesPerVertex + 6], vertices[i + entriesPerVertex + 7]);
		glm::vec2 uv3(vertices[i + entriesPerVertex * 2 + 6], vertices[i + entriesPerVertex * 2 + 7]);
		generateTangents(tangents, bitangents, pos1, pos2, pos3, uv1, uv2, uv3);
	}
	for (GLuint i = 0; i < nVertices; i++)
	{
		const glm::vec3 n(vertices[8 * i + 3], vertices[8 * i + 4], vertices[8 * i + 5]);
		const glm::vec3& t = tangents[i];

		// Gram-Schmidt orthogonalize
		tangents[i] = glm::normalize(t - n * glm::dot(n, t));

		// Calculate handedness
		GLfloat w = (glm::dot(glm::cross(n, t), bitangents[i]) < 0.0F) ? -1.0F : 1.0F;
		bitangents[i] = glm::normalize(glm::cross(n, t) * w);
	}
	//tangent attribute
	GLuint tbuffer;
	glGenBuffers(1, &tbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, tbuffer);
	glBufferData(GL_ARRAY_BUFFER, tangents.size() * sizeof(glm::vec3), &tangents[0], GL_STATIC_DRAW);
	glVertexAttribPointer(7,	//that's at location = 2 in the second version of the vertex shader
		3,	//3 values, vec3
		GL_FLOAT,	//Data type
		GL_FALSE,	//normalize(-1...1)?
		sizeof(glm::vec3),	//Stride
		(GLvoid*)(0));	//offset
	glEnableVertexAttribArray(7);
	//bitangents
	GLuint btbuffer;
	glGenBuffers(1, &btbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, btbuffer);
	glBufferData(GL_ARRAY_BUFFER, bitangents.size() * sizeof(glm::vec3), &bitangents[0], GL_STATIC_DRAW);
	glVertexAttribPointer(8,	//that's at location = 2 in the second version of the vertex shader
		3,	//3 values, vec3
		GL_FLOAT,	//Data type
		GL_FALSE,	//normalize(-1...1)?
		sizeof(glm::vec3),	//Stride
		(GLvoid*)(0));	//offset
	glEnableVertexAttribArray(8);

	// 4. Unbind VAO
	glBindVertexArray(0);

	nRenderingElemts = verticesSize / sizeof(GLfloat) / entriesPerVertex;
	indexedMode = false;
}

void Mesh::generateTangents(std::vector<glm::vec3>& tangents, std::vector<glm::vec3>& bitangents, const glm::vec3& pos1, const glm::vec3& pos2, const glm::vec3& pos3,
	const glm::vec2& uv1, const glm::vec2& uv2, const glm::vec2& uv3)
{
	glm::vec3 tangent;
	glm::vec3 bitangent;

	glm::vec3 edge1 = pos2 - pos1;
	glm::vec3 edge2 = pos3 - pos1;
	glm::vec2 deltaUV1 = uv2 - uv1;
	glm::vec2 deltaUV2 = uv3 - uv1;
	GLfloat f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

	tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
	tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
	tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
	tangent = glm::normalize(tangent);

	bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
	bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
	bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
	bitangent = glm::normalize(bitangent);

	//write them three times, because tangent and bitangent is one for all three vertices of the triangle
	tangents.push_back(tangent);
	tangents.push_back(tangent);
	tangents.push_back(tangent);
	bitangents.push_back(bitangent);
	bitangents.push_back(bitangent);
	bitangents.push_back(bitangent);
}

/**
	Creates VAO (and EBO) from C++ vectors, containing vertex data, and indexes for face building from these vertices
*/
void Mesh::loadVertices(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices)
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, verticesSize, &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

	// Vertex Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, position));
	// Vertex Normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, normal));
	// Vertex Texture Coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, textureCoords));
	// Vertex tangent Coords
	glEnableVertexAttribArray(7);
	glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, tangent));
	// Vertex bitangent Coords
	glEnableVertexAttribArray(8);
	glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, bitangent));

	glBindVertexArray(0);

	nRenderingElemts = indices.size();
	indexedMode = true;
}

/**
	Using shaderProgram, and Vector Array Object, make a draw call for OpenGL to render all our vertices
*/
void Mesh::render(GLuint shaderprogram)
{
	setUpMaterial(shaderprogram);

	glBindVertexArray(VAO);

	if ( indexedMode )
		glDrawElements(rMode, nRenderingElemts, GL_UNSIGNED_INT, 0);
	else
		glDrawArrays(rMode, 0, nRenderingElemts);

	//It is common practice to unbind OpenGL objects when we're done configuring them so we don't mistakenly (mis)configure them elsewhere. 
	glBindVertexArray(0);
	for (GLuint i = 0; i < textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

/**
Using shaderProgram, and Vector Array Object, make OpenGL to render all our vertices, several times in one call
*/
void Mesh::batchRender(GLuint shederprogram, GLuint numCalls)
{
	setUpMaterial(shederprogram);

	glBindVertexArray(VAO);

	if (indexedMode)
		glDrawElementsInstanced(rMode, nRenderingElemts, GL_UNSIGNED_INT, 0, numCalls);
	else
		glDrawArraysInstanced(rMode, 0, nRenderingElemts, numCalls);

	//It is common practice to unbind OpenGL objects when we're done configuring them so we don't mistakenly (mis)configure them elsewhere. 
	glBindVertexArray(0);
	for (GLuint i = 0; i < textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

/**
*	Sets up all the material data in the sahder
*/
void Mesh::setUpMaterial(GLuint shaderprogram)
{
	GLboolean normalAvailable = false;
	GLboolean parallaxAvailable = false;

	for (GLuint i = 0; i < textures.size(); i++)
	{
		std::string materialVar;

		if (textures[i].type == "texture_diffuse")
		{
			materialVar = "material.diffuse";
		}
		else if (textures[i].type == "texture_specular")
		{
			materialVar = "material.specular";
		}
		else if (textures[i].type == "texture_normal")
		{
			materialVar = "material.normalMap";
			normalAvailable = true;
		}
		else if (textures[i].type == "texture_depth")
		{
			materialVar = "material.depthMap";
			parallaxAvailable = true;
		}

		setModelTexture(shaderprogram, textures[i].id, materialVar.c_str(), i);
	}

	glUniform1i(glGetUniformLocation(shaderprogram, "material.normalAvailable"), normalAvailable);
	glUniform1i(glGetUniformLocation(shaderprogram, "material.parallaxAvailable"), parallaxAvailable);

	setModelMaterial(shaderprogram, 32.0f, textures[0].transparency);
}

/**
	Set texture object for the uniform sampler2D variable in the fragment shader.
	textureId 0..31
*/
void Mesh::setModelTexture(GLuint shaderProgram, GLuint texture, const GLchar* shaderVar, GLuint textureId)
{
	glActiveTexture(GL_TEXTURE0 + textureId);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(glGetUniformLocation(shaderProgram, shaderVar), textureId);
}

/**
	Sets material properties for a current model to be rendered (set prior to rendering)
*/
void Mesh::setModelMaterial(GLuint shaderProgram, GLfloat shininess, GLint isTransparent)
{
	GLint materialShininess = glGetUniformLocation(shaderProgram, "material.shininess");
	GLint materialTransparency = glGetUniformLocation(shaderProgram, "material.transparent");
	glUniform1f(materialShininess, shininess);
	glUniform1i(materialTransparency, isTransparent);
}
