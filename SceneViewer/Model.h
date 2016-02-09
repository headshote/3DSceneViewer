#ifndef MODEL
#define MODEL

#include <vector>
#include <iostream>
#include <memory>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <SOIL/SOIL.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "AsyncModelLoader.h"

namespace models
{
	GLuint loadTexture(const GLchar* filePath, const GLboolean isTransparent = false, const GLboolean gammaCorrect = false);

	class Model
	{
	public:
		explicit Model(const GLchar* filePath, const GLboolean useNormalMaps = false);
		explicit Model(renderables::Renderable* renderable, const std::string id);
		explicit Model(AsyncData& modelData);
		~Model();

		void setTranslation(const glm::vec3 translation);
		void setScale(const glm::vec3 scale);
		void setRotation(const glm::vec3 rotationAxis, const GLfloat angle);
		void setTransformation(const glm::mat4 transform);

		glm::vec3 getTranslation();
		glm::vec3 getScale();
		glm::vec3 getRotationAxis();
		GLfloat getRotationAngle();

		void setRendMode(GLenum mode);

		void drawCall(const GLuint shaderProgram);
		void drawOutlined(const GLuint renderShader, const GLuint outlineShader, const GLfloat outlineR = 0.43f, const GLfloat outlineG = 0.28f, const GLfloat outlineB = 0.06f);

		void scheduleRendering();
		void flushScheduledInstances();

		void batchRenderScheduledInstances(const GLuint shaderProgram);
		void batchRenderOutlined(const GLuint renderShader, const GLuint outlineShader, const GLfloat outlineR = 0.43f, const GLfloat outlineG = 0.28f, const GLfloat outlineB = 0.06f);

		void translateBy(const glm::vec3& translation);
		void scaleBy(const glm::vec3& scale);
		void rotateBy(const glm::vec3& rotationAxis, const GLfloat angle);

		std::string getID();
	private:
		std::vector<Texture> textures_loaded;

		std::vector<std::shared_ptr<renderables::Renderable>> meshes;

		glm::vec3 mTranslation;
		glm::vec3 mScale;
		glm::vec3 mRotationAxis;
		GLfloat mRotation;

		//for batching a lot of instances of the same model, but with different model transforms into one call
		GLuint instancedDataBufferId;	//here we'll store our model matrices for several instances, batched for rendering in OpenGL
		std::vector<glm::mat4> batchedInstanceTransforms;

		glm::mat4 transformation;	//model transformation

		std::string mId;

		void setUniformMaxtrix(const GLuint shaderProgram, const GLchar* uniformName, const glm::mat4& value);

		void initialize();
		void updateTransformation();

		Texture obtainTexture(const std::string& typeName, aiString& textureName, const std::string& texturePath);

		void loadModel(const std::string& filePath, const GLboolean useNormalMaps);
		void loadModel(AsyncData& modelData);
		void processNode(aiNode* node, const aiScene* scene, GLboolean useNormalMaps, const std::string& modelRootDir);
		renderables::Mesh* createMesh(aiMesh* mesh, const aiScene* scene, GLboolean useNormalMaps, const std::string& modelRootDir);
		std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName, const std::string& modelRootDir);
	};
}

#endif