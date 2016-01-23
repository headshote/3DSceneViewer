#pragma once

#include <vector>
#include <memory>
#include <string>

#include <thread>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace models
{
	struct AsyncData
	{
		std::string filePath;
		const aiScene* scene;
	};

	class AsyncModelLoader
	{
	public:
		~AsyncModelLoader();

		static std::shared_ptr<AsyncModelLoader> instance();

		void loadModel(const std::string& filePath, std::vector<AsyncData>* results);

	private:
		static std::shared_ptr<AsyncModelLoader> theInstance;

		static void asyncLoadCall(const std::string& filePath, std::vector<AsyncData>* results);

		std::thread t1;

		AsyncModelLoader();
	};
}
