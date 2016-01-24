#pragma once

#include <vector>
#include <memory>
#include <string>

#include <iostream>

#include <thread>
#include <mutex>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "DataStructures.hpp"

namespace models
{
	struct TextureData{
		std::string path;
		std::string typeName;
	};

	struct MeshData{
		std::vector<TextureData> diffuse;
		std::vector<TextureData> specular;
		std::vector<TextureData> normal;
		std::vector<Vertex> vertices;
		std::vector<GLuint> indices;
	};

	struct AsyncData
	{
		std::string filePath;
		std::vector<MeshData> meshes;
	};

	class AsyncModelLoader
	{
	public:
		~AsyncModelLoader();

		static std::shared_ptr<AsyncModelLoader> instance();

		void loadModel(const std::string* filePath, std::vector<AsyncData>* results);

		GLboolean isDone();

	private:
		static std::shared_ptr<AsyncModelLoader> theInstance;

		void asyncLoadCall(const std::string* fPath, std::vector<AsyncData>* results);

		std::mutex mtx;

		std::vector<std::thread> threads;
		std::vector<GLboolean> completedRequests;

		AsyncModelLoader();
	};

	void processNode(aiNode* node, const aiScene* scene, AsyncData& modelhData, const std::string& modelRootDir);
	void createMesh(aiMesh* mesh, const aiScene* scene, AsyncData& modelhData, const std::string& modelRootDir);
	std::vector<TextureData>* loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName, const std::string& modelRootDir);
}
