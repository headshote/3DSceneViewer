#include "Model.h"

using namespace models;

Model::Model(const GLchar* filePath, const GLboolean useNormalMaps) : mId(filePath)
{
	loadModel(filePath, useNormalMaps);

	initialize();
}

Model::Model(renderables::Renderable* renderable, const std::string id) : mId(id)
{
	meshes.push_back(std::shared_ptr<renderables::Renderable>(renderable));

	initialize();
}

Model::~Model()
{
}

void Model::setTranslation(const glm::vec3 translation)
{
	mTranslation = translation;
	updateTransformation();
}

void Model::setScale(const glm::vec3 scale)
{
	mScale = scale;
	updateTransformation();
}

void Model::setRotation(const glm::vec3 rotationAxis, const GLfloat angle)
{
	mRotation = angle;
	mRotationAxis = rotationAxis;
	updateTransformation();
}

void Model::setTransformation(const glm::mat4 transform)
{
	transformation = transform;
}

glm::vec3 Model::getTranslation()
{
	return mTranslation;
}

glm::vec3 Model::getScale()
{
	return mScale;
}

glm::vec3 Model::getRotationAxis()
{
	return mRotationAxis;
}

GLfloat Model::getRotationAngle()
{
	return mRotation;
}

void Model::initialize()
{
	mTranslation = glm::vec3(0.0f);
	mScale = glm::vec3(1.0f);
	mRotation = 0.0f;
	mRotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);
	updateTransformation();
}

void Model::updateTransformation()
{
	glm::mat4 trans;
	trans = glm::translate(trans, mTranslation);
	trans = glm::scale(trans, mScale);
	trans = glm::rotate(trans, mRotation, mRotationAxis);

	transformation = trans;
}

void Model::translateBy(const glm::vec3& translation)
{
	transformation = glm::translate(transformation, translation);
}

void Model::scaleBy(const glm::vec3& scale)
{
	transformation = glm::scale(transformation, scale);
}

void Model::rotateBy(const glm::vec3& rotationAxis, const GLfloat angle)
{
	transformation = glm::rotate(transformation, angle, rotationAxis);
}

void Model::drawCall(const GLuint shaderProgram)
{
	glUseProgram(shaderProgram);

	setUniformMaxtrix(shaderProgram, "model", transformation);

	for (GLuint i = 0; i < meshes.size(); i++)
		meshes[i]->drawCall(shaderProgram);
}

void Model::drawOutlined(const GLuint renderShader, const GLuint outlineShader, const GLfloat outlineR, const GLfloat outlineG, const GLfloat outlineB)
{
	//We first want to enable stencil testing and set the actions to take whenever any of the tests succeed or fail
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_DEPTH_TEST);
	glStencilOp(
		GL_KEEP,	//action to take if the stencil test fails.
		GL_KEEP,	//action to take if the stencil test passes, but the depth test fails.
		GL_REPLACE	//action to take if both the stencil and the depth test pass.
		);

	// 1. 
	//By using the GL_ALWAYS stencil testing function we make sure that each of the containers' fragments
	//update the stencil buffer with a stencil value of 1. 
	//Because the fragments always pass the stencil test, 
	//the stencil buffer is updated with the reference value wherever we've drawn them.
	glStencilFunc(
		GL_ALWAYS, //function (All fragments should update the stencil buffer)
		1,		   //ref value. The stencil buffer's content is compared to this value
		0xFF	   //bitmask (AND'ed with both the reference value and the stored stencil value before the test compares them)
		);
	glStencilMask(0xFF); // Enable writing to the stencil buffer

	glUseProgram(renderShader);
	drawCall(renderShader);

	glm::vec3 oldScale = getScale();

	//2.
	//Now that the stencil buffer is updated with 1s where the containers were drawn
	//we're going to draw the upscaled containers, but this time disabling writes to the stencil buffer
	glDisable(GL_DEPTH_TEST);
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilMask(0x00); // Disable writing to the stencil buffer

	glUseProgram(outlineShader);

	GLfloat yoffset = -0.15f;
	setScale(oldScale + 0.006f);
	translateBy(glm::vec3(0.0f, yoffset, 0.0f));
	glUniform3f(glGetUniformLocation(outlineShader, "lsColor"), outlineR, outlineG, outlineB);
	drawCall(outlineShader);

	//return things to the way they were
	translateBy(glm::vec3(0.0f, -yoffset, 0.0f));
	setScale(oldScale);
	glStencilMask(0xFF);
	glDisable(GL_STENCIL_TEST);
	glEnable(GL_DEPTH_TEST);
	glUseProgram(renderShader);
}

/**
	Schedules current transform matrix into data structure for instanced batch rendering
*/
void Model::scheduleRendering()
{
	glm::mat4 instanceModel(transformation);
 	batchedInstanceTransforms.push_back(instanceModel);
}

/**
	Writes all the scheduled transforms from the internal data structure (vector) to the OpenGL buffer
*/
void Model::flushScheduledInstances()
{
	for (GLuint i = 0; i < meshes.size(); i++)
	{
		GLuint VAO = meshes[i]->getVAO();
		// Vertex Buffer Object
		GLuint buffer;
		glBindVertexArray(VAO);
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER, batchedInstanceTransforms.size() * sizeof(glm::mat4), &batchedInstanceTransforms[0], GL_STATIC_DRAW);

		// Vertex Attributes
		//Split matrix into 4 vec4s, explanation:
		//The maximum amount of data allowed as a vertex attribute is equal to a vec4.
		//Because a mat4 is basically 4 vec4s, we have to reserve 4 vertex attributes for this specific matrix.
		//Because we assigned it a location of 3, the columns of the matrix will have vertex attribute locations of 3, 4, 5 and 6.
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (GLvoid*)0);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (GLvoid*)(sizeof(glm::vec4)));
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (GLvoid*)(2 * sizeof(glm::vec4)));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (GLvoid*)(3 * sizeof(glm::vec4)));

		//This function tells OpenGL when to update the content of a vertex attribute to the next element.
		//Its first parameter is the vertex attribute in question and the second parameter the attribute divisor.
		//By default the attribute divisor is 0 which tells OpenGL to update the content of the vertex attribute each iteration of the vertex shader.
		//By setting this attribute to 1 we're telling OpenGL that we want to update the content of the vertex attribute when we start to render a new instance. 
		//By setting it to 2 we'd update the content every 2 instances and so on.By setting the attribute divisor to 1 we're effectively telling 
		//OpenGL that the vertex attribute at attribute location 2 is an instanced array.
		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);

		glBindVertexArray(0);
	}
}

void Model::batchRenderScheduledInstances(const GLuint shaderProgram)
{
	glUseProgram(shaderProgram);

	for (GLuint i = 0; i < meshes.size(); i++)
		meshes[i]->drawBatch(shaderProgram, batchedInstanceTransforms.size());
}

void Model::batchRenderOutlined(const GLuint renderShader, const GLuint outlineShader, const GLfloat outlineR, const GLfloat outlineG, const GLfloat outlineB)
{
	//We first want to enable stencil testing and set the actions to take whenever any of the tests succeed or fail
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_DEPTH_TEST);
	glStencilOp(
		GL_KEEP,	//action to take if the stencil test fails.
		GL_KEEP,	//action to take if the stencil test passes, but the depth test fails.
		GL_REPLACE	//action to take if both the stencil and the depth test pass.
		);

	// 1. 
	//By using the GL_ALWAYS stencil testing function we make sure that each of the containers' fragments
	//update the stencil buffer with a stencil value of 1. 
	//Because the fragments always pass the stencil test, 
	//the stencil buffer is updated with the reference value wherever we've drawn them.
	glStencilFunc(
		GL_ALWAYS, //function (All fragments should update the stencil buffer)
		1,		   //ref value. The stencil buffer's content is compared to this value
		0xFF	   //bitmask (AND'ed with both the reference value and the stored stencil value before the test compares them)
		);
	glStencilMask(0xFF); // Enable writing to the stencil buffer

	glUseProgram(renderShader);
	batchRenderScheduledInstances(renderShader);

	glm::vec3 oldScale = getScale();

	//2.
	//Now that the stencil buffer is updated with 1s where the containers were drawn
	//we're going to draw the upscaled containers, but this time disabling writes to the stencil buffer
	glDisable(GL_DEPTH_TEST);
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilMask(0x00); // Disable writing to the stencil buffer

	glUseProgram(outlineShader);

	GLfloat yoffset = -0.15f;
	setScale(oldScale + 0.006f);
	translateBy(glm::vec3(0.0f, yoffset, 0.0f));
	glUniform3f(glGetUniformLocation(outlineShader, "lsColor"), outlineR, outlineG, outlineB);
	batchRenderScheduledInstances(outlineShader);

	//return things to the way they were
	translateBy(glm::vec3(0.0f, -yoffset, 0.0f));
	setScale(oldScale);
	glStencilMask(0xFF);
	glDisable(GL_STENCIL_TEST);
	glEnable(GL_DEPTH_TEST);
	glUseProgram(renderShader);
}

void Model::setUniformMaxtrix(const GLuint shaderProgram, const GLchar* uniformName, const glm::mat4& value)
{
	GLint uniform = glGetUniformLocation(shaderProgram, uniformName);
	glUniformMatrix4fv(uniform, 1,
		GL_FALSE,	//transpose?
		glm::value_ptr(value));
}

/**
	Fills meshes vector with renderable meshes, that contalin data about mesh textures (their texture objects and texture coordinates for vertices), 
	vertex coordinates	and vertex normals
*/
void Model::loadModel(const std::string& filePath, const GLboolean useNormalMaps)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace );

	if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
		return;
	}
	processNode(scene->mRootNode, scene, useNormalMaps, filePath.substr(0, filePath.find_last_of('/')));
}

void Model::processNode(aiNode* node, const aiScene* scene, const GLboolean useNormalMaps, const std::string& modelRootDir)
{
	// Process all the node's meshes (if any)
	for (GLuint i = 0; i < node->mNumMeshes; i++)
	{
		//Each node contains a set of mesh indices where each index points to a specific mesh,
		//located in the scene object.
		//We thus want to retrieve these mesh indices, retrieve each mesh, process each mesh
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(std::shared_ptr<renderables::Renderable>(createMesh(mesh, scene, useNormalMaps, modelRootDir)));
	}

	// Then do the same for each of its children
	for (GLuint i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene, useNormalMaps, modelRootDir);
	}
}

/**
	Creates Mesh object from an asimp mesh
*/
renderables::Mesh* Model::createMesh(aiMesh* mesh, const aiScene* scene, const GLboolean useNormalMaps, const std::string& modelRootDir)
{
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<Texture> textures;

	for (GLuint i = 0; i < mesh->mNumVertices; i++)
	{
		// Process vertex positions, normals and texture coordinates
		Vertex vertex; 
		glm::vec3 vector;

		vector.x = mesh->mTangents[i].x;
		vector.y = mesh->mTangents[i].y;
		vector.z = mesh->mTangents[i].z;
		vertex.tangent = vector;

		vector.x = mesh->mBitangents[i].x;
		vector.y = mesh->mBitangents[i].y;
		vector.z = mesh->mBitangents[i].z;
		vertex.bitangent = vector;

		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.position = vector;

		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;
		vertex.normal = vector;

		if (mesh->mTextureCoords[0]) // Does the mesh contain texture coordinates?
		{
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.textureCoords = vec;
		}
		else
			vertex.textureCoords = glm::vec2(0.0f, 0.0f);

		vertices.push_back(vertex);
	}

	// Process indices
	for (GLuint i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (GLuint j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	// Process materials, if they are present
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", modelRootDir);
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular", modelRootDir);
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

		if (useNormalMaps)
		{
			std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal", modelRootDir);
			textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
		}//piece of shit asimp won't work properly with mirrored normal maps, fuck this shit
	}

	return new renderables::Mesh(vertices, indices, textures);
}

/**
	Assumes that all the textures are in the root of the model folder
*/
std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName, const std::string& modelRootDir)
{
	std::vector<Texture> textures;
	
	for (GLuint i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString textureName;

		mat->GetTexture(type, i, &textureName);
		GLboolean skip = false;
		//First, check what's alredy loaded
		for (GLuint j = 0; j < textures_loaded.size(); j++)
		{
			if (textures_loaded[j].name == textureName)
			{
				textures.push_back(textures_loaded[j]);
				skip = true;
				break;
			}
		}

		//Only load the texture from file, if it hasn't been alredy
		//Add it to cache after
		if (!skip)
		{   // If texture hasn't been loaded already, load it
			Texture texture;
			texture.id = loadTexture((modelRootDir + "/" + std::string(textureName.C_Str())).c_str()/*, false, typeName == "texture_diffuse"*/);
			texture.type = typeName;
			texture.name = textureName;
			texture.transparency = 0;
			textures.push_back(texture);
			textures_loaded.push_back(texture);  // Add to loaded textures
		}
	}

	return textures;
}

void Model::initializeWithContext(ModelRenderingContext* context)
{
	context->applyContextStateToModel(*this);
}

void Model::renderWithContext(ModelRenderingContext* context, const GLuint shaderProgram, const GLuint batchShader)
{
	context->doRendering(*this, shaderProgram, batchShader);
}

std::string Model::getID()
{
	return mId;
}

/**
	Load texture from file, a non-member function, that is packaged along with the Model class
*/
GLuint models::loadTexture(const GLchar* filePath, const GLboolean isTransparent, const GLboolean gammaCorrect)
{
	int width, height;
	unsigned char* image = SOIL_load_image(filePath, &width, &height, 0, SOIL_LOAD_RGBA);

	//OpenGl texture object creation and binding, the usual
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	//create the texture
	glTexImage2D(GL_TEXTURE_2D,
		0,			//mipmap level 
		gammaCorrect ? GL_SRGB_ALPHA : GL_RGBA,
		width,
		height,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		image);
	//This will automatically generate all the required mipmaps for the currently bound texture
	glGenerateMipmap(GL_TEXTURE_2D);

	// Set the texture wrapping/filtering options (on the currently bound texture object)	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, isTransparent ? GL_CLAMP_TO_EDGE : GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, isTransparent ? GL_CLAMP_TO_EDGE : GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//free, unbind texture
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	return texture;
}

