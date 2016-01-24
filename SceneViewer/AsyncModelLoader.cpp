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
	for (GLuint i = 0; i < threads.size(); ++i)
		threads[i].join();
}

GLboolean AsyncModelLoader::isDone()
{
	return threads.size() == completedRequests.size();
}

void AsyncModelLoader::loadModel(const std::string* filePath, std::vector<AsyncData>* results)
{
	threads.push_back(std::thread{ std::bind(&AsyncModelLoader::asyncLoadCall, this, filePath, results) });
}

void AsyncModelLoader::asyncLoadCall(const std::string* fPath, std::vector<AsyncData>* results)
{
	Assimp::Importer importer;

	const std::string& filePath = std::string(*fPath);

	const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
		return;
	}

	AsyncData meshData;
	meshData.filePath = filePath;
	meshData.vertices = *(new std::vector<std::vector<Vertex>>);
	meshData.indices = *(new std::vector<std::vector<GLuint>>);

	processNode(scene->mRootNode, scene, meshData, filePath.substr(0, filePath.find_last_of('/')));

	std::unique_lock<std::mutex> lck(mtx);
	results->push_back(meshData);
	completedRequests.push_back(true);
}

void models::processNode(aiNode* node, const aiScene* scene, AsyncData& meshData, const std::string& modelRootDir)
{
	// Process all the node's meshes (if any)
	for (GLuint i = 0; i < node->mNumMeshes; i++)
	{
		//Each node contains a set of mesh indices where each index points to a specific mesh,
		//located in the scene object.
		//We thus want to retrieve these mesh indices, retrieve each mesh, process each mesh
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

		createMesh(mesh, scene, meshData, modelRootDir);
	}

	// Then do the same for each of its children
	for (GLuint i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene, meshData, modelRootDir);
	}
}

void models::createMesh(aiMesh* mesh, const aiScene* scene, AsyncData& meshData, const std::string& modelRootDir)
{
	std::vector<Vertex>* vertics = new std::vector<Vertex>();
	std::vector<GLuint>* indics = new std::vector<GLuint>;

	std::vector<Vertex>& vertices = *vertics;
	std::vector<GLuint>& indices = *indics;

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

		std::vector<TextureData>& diffuseMaps = *loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", meshData, modelRootDir);
		std::vector<TextureData>& specularMaps = *loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular", meshData, modelRootDir);
		meshData.diffuse = diffuseMaps;
		meshData.specular = specularMaps;
	}

	meshData.vertices.push_back( vertices);
	meshData.indices.push_back(indices);
}

std::vector<TextureData>* models::loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName, AsyncData& meshData, const std::string& modelRootDir)
{
	std::vector<TextureData>* textures = new std::vector<TextureData>;

	for (GLuint i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString textureName;

		mat->GetTexture(type, i, &textureName);
		
		TextureData txtrData{
			modelRootDir + "/" + std::string(textureName.C_Str()),
			std::string(typeName)
		};
		textures->push_back(txtrData);		
	}

	return textures;
}