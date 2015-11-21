#pragma once

#include <vector>
#include <iostream>
#include <memory>

#include <SOIL/SOIL.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"

class Model
{
public:
	static GLuint loadTexture(const GLchar* filePath, GLboolean isTransparent = false, GLboolean gammaCorrect = false);

	void setTranslation(glm::vec3 translation);
	void setScale(glm::vec3 scale);
	void setRotation(glm::vec3 rotationAxis, GLfloat angle);

	glm::vec3 getTranslation();
	glm::vec3 getScale();
	glm::vec3 getRotationAxis();
	GLfloat getRotationAngle();

	Model(const GLchar* filePath, GLboolean useNormalMaps = false);
	Model(Renderable* renderable);
	~Model();

	void drawCall(GLuint shaderProgram);
	void drawOutlined(GLuint renderShader, GLuint outlineShader, GLfloat outlineR = 0.43f, GLfloat outlineG = 0.28f, GLfloat outlineB = 0.06f);

	void scheduleRendering();
	void flushScheduledInstances();
	void batchRenderScheduledInstances(GLuint shaderProgram);
	void batchRenderOutlined(GLuint renderShader, GLuint outlineShader, GLfloat outlineR = 0.43f, GLfloat outlineG = 0.28f, GLfloat outlineB = 0.06f);

	void translateBy(const glm::vec3& translation);
	void scaleBy(const glm::vec3& scale);
	void rotateBy(const glm::vec3& rotationAxis, GLfloat angle);

	void dispose();
protected:
	std::vector<Texture> textures_loaded;

	std::vector<std::shared_ptr<Renderable>> meshes;

	glm::vec3 mTranslation;
	glm::vec3 mScale;
	glm::vec3 mRotationAxis;
	GLfloat mRotation;

	//for batching a lot of instances of the same model, but with different model transforms into one call
	GLuint instancedDataBufferId;	//here we'll store our model matrices for several instances, batched for rendering in OpenGL
	std::vector<glm::mat4> batchedInstanceTransforms;

	glm::mat4 transformation;	//model transformation
private:
	void setUniformMaxtrix(GLuint shaderProgram, GLchar* uniformName, const glm::mat4& value);

	void initialize();
	void updateTransformation();

	void loadModel(const std::string& filePath, GLboolean useNormalMaps);
	void processNode(aiNode* node, const aiScene* scene, GLboolean useNormalMaps, const std::string& modelRootDir);
	Mesh* createMesh(aiMesh* mesh, const aiScene* scene, GLboolean useNormalMaps, const std::string& modelRootDir);
	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName, const std::string& modelRootDir);
};

