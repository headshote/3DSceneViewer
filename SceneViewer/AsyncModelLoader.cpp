#include "AsyncModelLoader.h"

using namespace models;

/*static*/ std::shared_ptr<AsyncModelLoader> AsyncModelLoader::theInstance;

/*static*/ std::shared_ptr<AsyncModelLoader> AsyncModelLoader::instance()
{
	if (!theInstance.get())
		theInstance.reset(new AsyncModelLoader());
	return theInstance;
}

AsyncModelLoader::AsyncModelLoader()
{

}


AsyncModelLoader::~AsyncModelLoader()
{
	t1.join();
}

void AsyncModelLoader::loadModel(const std::string& filePath, std::vector<AsyncData>* results)
{
	t1 = std::thread{ std::bind(asyncLoadCall, filePath, results) };
}

/*static*/ void AsyncModelLoader::asyncLoadCall(const std::string& filePath, std::vector<AsyncData>* results)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	results->push_back(AsyncData{
		filePath,
		scene
	});
}
